#pragma once

#include <esp_err.h>
#include <esp_log.h>
#include <mdns.h>

#include "network/hostname.h"

/**
 * Multicast DNS manager
 */
class MulticastDns {
    public:
        /**
         * Constructor
         * @param hostnameManager Hostname manager
         */
        explicit MulticastDns(HostnameManager *hostnameManager);
    private:
        /// Hostname mananager
        HostnameManager *hostnameManager;
};
