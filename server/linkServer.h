#pragma once

#include <helpers.h>

constexpr uint32_t MAX_LINK_SLOTS = 4;

class LinkServer_t
{
public:

	// Starts listening on the given port (binds on any local address).
	// NET_CreateServer itself is non-blocking -- this returns immediately,
	// clients connect asynchronously via update()'s NET_AcceptClient calls.
	FLAG start(uint16_t port);

	// Closes every client socket and the server socket, calls NET_Quit().
	// Safe to call even if start() was never called or already stopped.
	void stop();

	// Pumps the whole server. Must never block:
	//   1. NET_AcceptClient() -- picks up at most one new connection per
	//      call (non-blocking; returns immediately if none pending).
	//   2. For each connected client: drain whatever bytes are available
	//      into that client's accumulation buffer, process any complete
	//      LinkMessage_t found there (looping in case multiple arrived
	//      in one read -- same TCP partial-message reality as the client
	//      side).
	// Call this in a tight loop from main() -- this binary's only job is
	// to run this repeatedly.
	void update();

	uint32_t getConnectedClientCount() const;

private:

	struct ClientSlot
	{
		NET_StreamSocket* socket = nullptr;
		uint32_t slotID = ZERO;
		FLAG connected = NO;
		std::vector<BYTE> rxAccumulator;
	};

	NET_Server* server = nullptr;
	ClientSlot clients[MAX_LINK_SLOTS];

	void acceptNewClients();
	void serviceClient(ClientSlot& client);
	void processAccumulatedMessages(ClientSlot& client);
	void handleMessage(ClientSlot& sender, const LinkMessage_t& message);
	void disconnectClient(ClientSlot& client);

	// Sent to EVERY currently-connected client (including whoever just
	// joined) any time membership changes. This is the single source of
	// truth clients build their roster from -- no client ever assembles
	// this picture itself from a stream of join/leave deltas.
	uint32_t computeConnectedBitmask() const;
	void broadcastRoster();

	// Slot 0 <-> slot 1 mirror: whatever one sends, the other receives.
	// This is the whole "relay policy" for 2 players -- the 4-player
	// master/slave fan-out logic is a deliberately separate follow-up,
	// not something to half-build in here now.
	void relaySerialByte(ClientSlot& sender, const LinkMessage_t& message);

	FLAG findFreeSlot(uint32_t* outSlotID);
};