#include "linkServer.h"

FLAG LinkServer_t::start(uint16_t port)
{
	if (!NET_Init())
	{
		LOG("LinkServer: NET_Init failed: %s", SDL_GetError());
		RETURN FAILURE;
	}

	// nullptr address = bind on any local interface.
	server = NET_CreateServer(nullptr, port, 0);
	if (server == nullptr)
	{
		LOG("LinkServer: NET_CreateServer failed: %s", SDL_GetError());
		NET_Quit();
		RETURN FAILURE;
	}

	LOG("LinkServer: listening on port %u", port);

	RETURN SUCCESS;
}

void LinkServer_t::stop()
{
	for (auto& client : clients)
	{
		if (client.connected == YES)
		{
			disconnectClient(client);
		}
	}

	if (server != nullptr)
	{
		NET_DestroyServer(server);
		server = nullptr;
	}

	NET_Quit();
}

void LinkServer_t::update()
{
	if (server == nullptr)
	{
		RETURN;
	}

	acceptNewClients();

	for (auto& client : clients)
	{
		if (client.connected == YES)
		{
			serviceClient(client);
		}
	}
}

void LinkServer_t::acceptNewClients()
{
	NET_StreamSocket* incoming = nullptr;

	// NET_AcceptClient is non-blocking: returns true immediately whether
	// or not a connection was actually accepted -- check the out-pointer,
	// not just the return value, to know if there's really a new client.
	if (!NET_AcceptClient(server, &incoming))
	{
		LOG("LinkServer: NET_AcceptClient error: %s", SDL_GetError());
		RETURN;
	}

	if (incoming == nullptr)
	{
		RETURN; // nothing pending, nothing to do
	}

	uint32_t freeSlot = ZERO;
	if (findFreeSlot(&freeSlot) == FAILURE)
	{
		LOG("LinkServer: rejecting connection, no free slots");
		NET_DestroyStreamSocket(incoming);
		RETURN;
	}

	clients[freeSlot].socket = incoming;
	clients[freeSlot].slotID = freeSlot;
	clients[freeSlot].connected = YES;
	clients[freeSlot].rxAccumulator.clear();

	LOG("LinkServer: client connected, assigned slot %u", freeSlot);

	LinkMessage_t assign{};
	assign.type = LinkMsg::SERVER_ASSIGN_SLOT;
	assign.slotID = freeSlot;
	assign.serialByte = ZERO;
	assign.sequence = ZERO;
	NET_WriteToStreamSocket(clients[freeSlot].socket, &assign, sizeof(assign));

	// One atomic snapshot to everyone (including the new client) instead
	// of a targeted "someone joined" notice to everyone else. Every
	// client -- old and new alike -- ends up computing its role from the
	// exact same fact.
	broadcastRoster();
}

uint32_t LinkServer_t::computeConnectedBitmask() const
{
	uint32_t bitmask = ZERO;
	for (const auto& client : clients)
	{
		if (client.connected == YES)
		{
			bitmask |= (1u << client.slotID);
		}
	}
	RETURN bitmask;
}

void LinkServer_t::broadcastRoster()
{
	uint32_t bitmask = computeConnectedBitmask();

	LinkMessage_t roster{};
	roster.type = LinkMsg::SERVER_ROSTER;
	roster.slotID = ZERO; // unused for this message type
	roster.serialByte = ZERO;
	roster.sequence = ZERO;
	roster.connectedSlotBitmask = bitmask;

	for (auto& client : clients)
	{
		if (client.connected == YES)
		{
			NET_WriteToStreamSocket(client.socket, &roster, sizeof(roster));
		}
	}
}

FLAG LinkServer_t::findFreeSlot(uint32_t* outSlotID)
{
	for (uint32_t i = 0; i < MAX_LINK_SLOTS; i++)
	{
		if (clients[i].connected == NO)
		{
			*outSlotID = i;
			RETURN SUCCESS;
		}
	}
	RETURN FAILURE;
}

void LinkServer_t::serviceClient(ClientSlot& client)
{
	BYTE readScratch[256];
	int bytesRead = NET_ReadFromStreamSocket(client.socket, readScratch, sizeof(readScratch));

	if (bytesRead > 0)
	{
		client.rxAccumulator.insert(client.rxAccumulator.end(), readScratch, readScratch + bytesRead);
		processAccumulatedMessages(client);
	}
	else if (bytesRead < 0)
	{
		LOG("LinkServer: read error on slot %u, disconnecting: %s", client.slotID, SDL_GetError());
		disconnectClient(client);
	}
	// bytesRead == 0: nothing available this call, nothing to do.
}

void LinkServer_t::processAccumulatedMessages(ClientSlot& client)
{
	while (client.rxAccumulator.size() >= sizeof(LinkMessage_t))
	{
		LinkMessage_t message;
		memcpy(&message, client.rxAccumulator.data(), sizeof(LinkMessage_t));

		handleMessage(client, message);

		client.rxAccumulator.erase(client.rxAccumulator.begin(), client.rxAccumulator.begin() + sizeof(LinkMessage_t));
	}
}

void LinkServer_t::handleMessage(ClientSlot& sender, const LinkMessage_t& message)
{
	switch (message.type)
	{
	case LinkMsg::SERIAL_BYTE_REQUEST:
	case LinkMsg::SERIAL_BYTE_REPLY:
	{
		// The server doesn't need to distinguish request vs. reply -- it's
		// a dumb relay either way. Forward verbatim, untouched, with the
		// ORIGINAL sender's slotID intact.
		relaySerialByte(sender, message);
		BREAK;
	}
	case LinkMsg::TEST_PACKET:
	{
		// message.slotID is client-supplied, over the network -- never
		// trust it as a bare array index without bounds-checking first.
		if (message.slotID >= MAX_LINK_SLOTS || clients[message.slotID].connected == NO)
		{
			LOG("LinkServer: TEST_PACKET with invalid/stale slotID %u, ignoring", message.slotID);
			BREAK;
		}

		LinkMessage_t assign{};
		assign.type = LinkMsg::TEST_PACKET;
		assign.slotID = message.slotID;
		assign.serialByte = message.serialByte;
		assign.sequence = message.sequence + ONE;
		NET_WriteToStreamSocket(clients[message.slotID].socket, &assign, sizeof(assign));
		BREAK;
	}
	case LinkMsg::CLIENT_HELLO:
	case LinkMsg::HEARTBEAT:
	default:
		// CLIENT_HELLO is currently a no-op on receipt -- slot assignment
		// already happened synchronously in acceptNewClients(). Revisit
		// if you want the client to wait for an explicit ack before
		// treating itself as READY instead of assuming SERVER_ASSIGN_SLOT
		// arriving is enough.
		BREAK;
	}
}

void LinkServer_t::relaySerialByte(ClientSlot& sender, const LinkMessage_t& message)
{
	for (auto& other : clients)
	{
		if (other.connected == YES && other.slotID != sender.slotID)
		{
			// Forward as-is, just stamped with the ORIGINAL sender's
			// slotID so the receiving client knows who it came from --
			// don't overwrite slotID with the receiver's own.
			NET_WriteToStreamSocket(other.socket, &message, sizeof(message));
		}
	}
}

void LinkServer_t::disconnectClient(ClientSlot& client)
{
	NET_DestroyStreamSocket(client.socket);
	client.socket = nullptr;
	client.connected = NO;
	client.rxAccumulator.clear();

	// client.connected is already NO by the time this runs, so
	// computeConnectedBitmask() inside here correctly excludes it.
	broadcastRoster();
}

uint32_t LinkServer_t::getConnectedClientCount() const
{
	uint32_t count = ZERO;
	for (const auto& client : clients)
	{
		if (client.connected == YES)
		{
			count++;
		}
	}
	RETURN count;
}