#pragma once

#pragma region INCLUDES
//
#include "helpers.h"
//
#include "bios.h"
//
#include "cheats.h"
#pragma endregion INCLUDES

#pragma region MACROS
#ifndef __RPI_PICO__
#define PAUSE_OR_RESUME(button)\
if (ImGui::IsKeyPressed(button) == true)\
{\
	bEmulationRun = !bEmulationRun;\
}

#define MUTE_OR_UNMUTE(button)\
if (ImGui::IsKeyPressed(button) == true)\
{\
	_MUTE_AUDIO = !_MUTE_AUDIO;\
}
#endif // !__RPI_PICO__
#pragma endregion MACROS

#pragma region GLOBAL_INFRASTRUCTURE_DECLARATIONS
extern uint32_t nEmulationInstanceID;
extern uint32_t _XFPS;

extern FLAG bWaitingForConnection;

extern debugConfig_t debugConfig;

extern std::string exeName;

extern std::string _BIOS_LOCATION;
extern std::string _CONFIG_LOCATION;
extern std::string _EXE_LOCATION;
extern std::string _SAVE_LOCATION;

extern FLAG _ENABLE_AUDIO;
extern FLAG _MUTE_AUDIO;
extern FLAG _RUN_DISASSEMBLER;
extern FLAG _ENABLE_FRAME_LIMIT;
extern FLAG _ENABLE_QUICK_SAVE;
extern FLAG _ENABLE_BESS_FORMAT;
extern FLAG _ENABLE_REWIND;
extern SBYTE _SET_PPU_VERSION;
extern uint32_t _REWIND_BUFFER_SIZE;
extern int32_t _TEST_NUMBER;
extern FLAG _ENABLE_ACCURATE_INPUT_SAMPLING;

extern float _ACTUAL_FPS;

#ifndef __RPI_PICO__
extern uint32_t frame_buffer;
extern uint32_t masquerade_texture;
extern uint32_t shaderProgramBasic;
extern uint32_t shaderProgramBlend;
extern uint32_t fullscreenVAO;
extern uint32_t fullscreenVBO;
extern uint32_t FRAME_BUFFER_SCALE;

using InputHintCallback = std::function<void()>;
#endif // !__RPI_PICO__

#pragma endregion GLOBAL_INFRASTRUCTURE_DECLARATIONS

#ifndef __EMSCRIPTEN__
#pragma region NETWORK
class abstractEmulationLinkSession_t
{
public:

	// Resolves + connects to hostAddress:port. This DOES block briefly
	// (NET_WaitUntilResolved / NET_WaitUntilConnected with a short
	// timeout) -- acceptable here because connect() is a rare,
	// user-initiated action (clicking "Connect"), not something called
	// from the serialTick hot path. Never call this from serialTick().
	FLAG connect(const char* hostAddress, uint16_t port);
	void disconnect();

	// Pumps socket I/O. Fully generic -- no knowledge of GB/NES/anything
	// else, so it's safe to call from serialTick(), the ImGui per-frame
	// loop, a future NES core, or with nothing running at all (test
	// mode). Call this as OFTEN AS CONVENIENT from every context that
	// might care about connection state -- this is the fix for the
	// deadlock class of bug: an inbound request from the peer must get
	// drained and answered even while THIS side is idle, mid-negotiation,
	// or has its own request outstanding. Self-throttled internally to
	// real socket reads at most once per NETWORK_POLL_INTERVAL_MS, so
	// calling this from multiple places/every tick is cheap, not
	// redundant work -- more callers just means less worst-case latency
	// before something gets noticed and processed.
	void update();

	// Cheap setter: whichever system is currently driving a byte-oriented
	// serial exchange (GB's serialTick(), eventually NES) calls this with
	// its current outgoing byte whenever it has one fresh. update() reads
	// it (via localReplyByte) ONLY when it needs to auto-reply to an
	// inbound SERIAL_BYTE_REQUEST -- see handleMessage(). If nothing ever
	// calls this (e.g. test mode, no ROM loaded), it stays at its
	// idle-line default and that's fine: nothing GB-shaped should be
	// sending SERIAL_BYTE_REQUEST in that state anyway.
	void setLocalReplyByte(BYTE replyByte);

	LinkConnectionState getConnectionState() const;
	uint32_t getAssignedSlot() const;
	uint32_t getPeerCount() const;

	// MASTER-side. Call once per tick from serialTick()'s master branch
	// (throttled, TRANSFER_ENABLE-gated, as today). Does NOT call
	// update() itself anymore -- update() runs unconditionally elsewhere
	// now, so this only needs to check whether a matching reply has
	// already arrived.
	FLAG tickSerialLink(BYTE outgoingByte, BYTE* outReceivedByte);

	// SLAVE-side. Call once per tick from serialTick()'s slave branch.
	// No longer takes an outgoingByte parameter -- any reply to an
	// inbound request was already sent proactively from update() using
	// the freshest SB available at the moment the request arrived. This
	// only reports whether such an exchange completed, and what byte the
	// peer's request carried (which becomes OUR received SB value, same
	// as real hardware: the master's outgoing byte is what the slave
	// receives).
	FLAG tickSerialLinkAsSlave(BYTE* outReceivedByte);

	void processAccumulatedMessages();
	void handleMessage(const LinkMessage_t& message);

	// channel defaults to DIRECT_LINK -- every EXISTING call site
	// (tickSerialLink()) is a direct-link exchange and needs no changes.
	// Only the future adapter module needs to explicitly pass ADAPTER.
	FLAG beginByteTransfer(BYTE byteToSend, LinkChannel channel = LinkChannel::DIRECT_LINK);

	// Few additional public getters and setters
	FLAG isTransferPending() const;
	FLAG hasReceivedByte() const;
	BYTE getLastReceivedByte() const;
	void clearUnclaimedReceivedByte();
	void clearTransferPending();

	// Slave-role queue: incoming requests from a peer acting as master.
	// The reply to each of these was ALREADY sent proactively inside
	// handleMessage()'s SERIAL_BYTE_REQUEST case, the instant it arrived
	// -- that immediacy is load-bearing (see the class-level comment on
	// update()). These accessors are pure consumption, nothing more; if
	// you find yourself wanting to send anything from the call site that
	// drains these, that's a sign the reply-on-arrival design is being
	// bypassed again.
	FLAG hasPendingSlaveByte() const;
	BYTE popPendingSlaveByte();

	NET_StreamSocket* getSocket() const;

	// Per the HW-accurate rule we settled on: the DMG-07 is powered
	// entirely through Player 1's cable (slot 0). No slot 0 present means
	// no adapter, full stop -- not "someone else takes over." So this
	// deliberately checks bit 0 explicitly, not just a raw peer count,
	// even though in practice bit 0 being set already implies it's the
	// lowest possible bit.
	FLAG isFourPlayerAdapterActive() const;

	// True only for the slot 0 instance, and only while the adapter is
	// actually active per the above. Every OTHER connected slot's local
	// copy of this same check correctly evaluates to NO for itself --
	// there's no election, no negotiation, just each client checking
	// "is my own assignedSlot 0" against the one shared roster fact.
	FLAG isAdapterLeader() const;

	// Testing and debugging
	FLAG sendTestPacket(BYTE byteToSend);

	FLAG hasReceivedTestPacket() const;
	BYTE getLastReceivedTestPacket() const;
	void clearReceivedTestPacket();

	const char* getLastError() const;
	void clearLastError();

protected:

	NET_StreamSocket* socket = nullptr;
	LinkConnectionState connectionState = LinkConnectionState::DISCONNECTED;
	uint32_t assignedSlot = ZERO;
	uint32_t peerCount = ZERO;

	// TCP gives no message-boundary guarantee: one NET_ReadFromStreamSocket
	// call might return half a LinkMessage_t, or three and a half of them.
	// This buffer accumulates raw bytes across calls; processAccumulatedMessages()
	// only consumes/dispatches once at least sizeof(LinkMessage_t) bytes are
	// present, looping in case multiple messages arrived in one read.
	std::vector<BYTE> rxAccumulator;

	// update() can be called from tickSerialLink()/tickSerialLinkAsSlave()
	// at raw CPU clock rate (up to 4.19MHz for the slave path, since it
	// isn't gated by serialTick()'s master-only throttle). A real socket
	// syscall on every one of those calls would be enormous, needless
	// overhead -- real network RTT is milliseconds at best, so polling
	// more often than ~1ms buys nothing. Wall-clock-based (not tick-count
	// based) so this stays correct regardless of GB normal/double-speed
	// mode.
	uint64_t lastNetworkPollMs = ZERO;
	static constexpr uint64_t NETWORK_POLL_INTERVAL_MS = 1;

	FLAG transferPending = NO;
	uint32_t pendingSequence = ZERO;
	FLAG hasReply = NO;
	BYTE replyByte = ZERO;

	// Idle-line default (0xFF), same convention as the old
	// receiveOverSerialLink()'s *bitReceived = ONE. Only meaningful once
	// something calls setLocalReplyByte() -- in test mode with no ROM
	// loaded, this just never gets touched, which is correct: nothing
	// should be sending us a SERIAL_BYTE_REQUEST in that state to begin
	// with.
	BYTE localReplyByte = 0xFF;

	// --- Slave-role state: peer requests we've already auto-replied to,
	// waiting for the LOCAL slave branch to pick them up. A real queue,
	// not a single slot -- more than one inbound request CAN arrive
	// before serialTick's slave branch next runs (that single-slot
	// overwrite was bug #2), so this must not silently drop any.
	std::deque<BYTE> pendingSlaveBytes;

	uint32_t txSequenceCounter = ZERO;

	// Updated only when a SERVER_ROSTER message arrives -- see
	// handleMessage(). Both the master and slave branches of serialTick()
	// query the SAME cached value here; this is one fact read from two
	// call sites, not two independently-computed answers that could
	// disagree.
	uint32_t connectedSlotBitmask = ZERO;

	// For testing/debugging only: a single-byte packet that can be sent
	BYTE lastReceivedTestPacket = ZERO;
	FLAG hasUnclaimedTestPacket = NO;

	std::string lastError;
};
#pragma endregion NETWORK
#endif

#pragma region CORE
class abstractEmulation_t
{

#pragma region INFRASTRUCTURE_DECLARATIONS
public:

	EMULATION_ID myID = EMULATION_ID::DEFAULT_ID;

	const uint32_t screen_height = 500;
	const uint32_t screen_width = 800;
	const uint32_t pixel_height = 1;
	const uint32_t pixel_width = 1;
	const float myFPS = DEFAULT_FPS;
	const char* NAME = "Masquerade";

#pragma endregion INFRASTRUCTURE_DECLARATIONS

#pragma region EMULATION_DECLARATIONS
#pragma endregion EMULATION_DECLARATIONS

#pragma region INFRASTRUCTURE_DEFINITIONS
public:

	abstractEmulation_t() {};

	virtual ~abstractEmulation_t() {};

	virtual void setupTheCoreOfEmulation(void* masqueradeInstance = nullptr, void* audio = nullptr, void* input = nullptr, void* network = nullptr) = 0;

public:

	float getVersion()
	{
		RETURN VERSION;
	}
	virtual const char* getEmulatorName()
	{
		RETURN NAME;
	}
	virtual float getEmulationFPS()
	{
		RETURN myFPS;
	}
	virtual float getEmulationVolume()
	{
		RETURN EMULATION_VOLUME;
	}
	virtual void setEmulationVolume(float volume)
	{
		MASQ_UNUSED(volume);
	}
	virtual uint32_t getScreenWidth() 
	{
		RETURN screen_width;
	}
	virtual uint32_t getScreenHeight() 
	{
		RETURN screen_height;
	}
	virtual uint32_t getPixelWidth() 
	{
		RETURN pixel_width;
	}
	virtual uint32_t getPixelHeight() 
	{
		RETURN pixel_height;
	}
	virtual uint32_t getTotalScreenWidth()
	{
		RETURN this->screen_width;
	}
	virtual uint32_t getTotalScreenHeight()
	{
		RETURN this->screen_height;
	}
	virtual uint32_t getTotalPixelWidth()
	{
		RETURN this->pixel_width;
	}
	virtual uint32_t getTotalPixelHeight()
	{
		RETURN this->pixel_height;
	}
	virtual void setScreenWidth(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setScreenHeight(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setPixelWidth(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setPixelHeight(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setTotalScreenWidth(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setTotalScreenHeight(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setTotalPixelWidth(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setTotalPixelHeight(uint32_t size)
	{
		MASQ_UNUSED(size);
	}
	virtual void setEmulationID(EMULATION_ID ID)
	{
		myID = ID;
	}
	virtual EMULATION_ID getEmulationID()
	{
		RETURN myID;
	}

	virtual FLAG getRomLoadedStatus() = 0;
	virtual FLAG loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom) = 0;
	virtual void dumpRom() = 0;
#pragma endregion INFRASTRUCTURE_DEFINITIONS

#pragma region EMULATOR_DEFINITIONS
public:

	virtual FLAG saveState(uint8_t id = 0) = 0;
	virtual FLAG loadState(uint8_t id = 0) = 0;
	virtual FLAG fillGamePlayStack() = 0;
	virtual FLAG rewindGamePlay() = 0;

	virtual FLAG runEmulationAtHostRate(uint32_t currentFrame) = 0;
	virtual FLAG runEmulationLoopAtHostRate(uint32_t currentFrame) = 0;
	virtual FLAG runEmulationAtFixedRate(uint32_t currentFrame) = 0;
	virtual FLAG runEmulationLoopAtFixedRate(uint32_t currentFrame) = 0;

	virtual FLAG onKeyEvent(EmuKey key, EmuKeyAction action) = 0;

	virtual FLAG initializeEmulator() = 0;
	virtual void destroyEmulator() = 0;

#ifndef __RPI_PICO__
public:
	void setInputHintCallback(InputHintCallback cb) 
	{
		inputHintCallback = cb;
	}
protected:
	InputHintCallback inputHintCallback = nullptr;
#endif // !__RPI_PICO__

#if !defined(__RPI_PICO__) && !defined(__EMSCRIPTEN__)
public:
	static abstractEmulationLinkSession_t& getLinkSession()
	{
		RETURN linkSession;
	}
protected:
	inline static abstractEmulationLinkSession_t linkSession;
#endif // !__RPI_PICO__

#pragma endregion EMULATOR_DEFINITIONS

#pragma region CORE_DEFINITIONS
	virtual void sendBiosToEmulator(bios_t* bios) = 0;
#pragma endregion CORE_DEFINITIONS
};
#pragma endregion CORE