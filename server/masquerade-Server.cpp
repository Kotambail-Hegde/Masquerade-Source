#include "linkServer.h"

// Logging
MAP64 ENABLE_LOGS = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000;

int main() 
{
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY);
	SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);

	constexpr uint16_t DEFAULT_PORT = 7777;

	LinkServer_t server;
	if (server.start(DEFAULT_PORT) == FAILURE)
	{
		LOG("Failed to start server, exiting.");
		RETURN INVALID;
	}

	LOG("masquerade-server running. Press Ctrl+C to quit.");

	while (true)
	{
		server.update();

		// Not a busy-loop: sleep a little between polls. Doesn't need to
		// be fast -- this only needs to be faster than real network RTT,
		// same reasoning as the client-side NETWORK_POLL_INTERVAL_MS.
		SDL_Delay(1);
	}

	// Unreachable with the loop above as-is -- worth adding a real
	// shutdown signal (Ctrl+C handler) before this matters, but leaving
	// this here as the obvious place stop() belongs once you do.
	server.stop();
	RETURN 0;

	NET_Quit();
}
