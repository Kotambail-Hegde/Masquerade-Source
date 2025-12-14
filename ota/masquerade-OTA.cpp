#include "ota.h"

// Logging
MAP64 ENABLE_LOGS = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000;

int main() {

    SETBIT(ENABLE_LOGS, LOG_VERBOSITY);
    SETBIT(ENABLE_LOGS, LOG_VERBOSITY_INFO);

    // Create an instance of your OTA class
    ota_t otaManager;

    // Prepare a property tree
    boost::property_tree::ptree pt;

    try
    {
        // Check for updates
        if (otaManager.checkForUpdates(pt))
        {
            // Attempt upgrade
            if (isOtaPossible == YES)
            {
                otaManager.upgrade(pt);
            }
        }
    }
    catch (const std::exception& e)
    {
        FATAL("Exception: %s\n", e.what());
        RETURN 1;
    }

    PAUSE;
    RETURN 0;
}
