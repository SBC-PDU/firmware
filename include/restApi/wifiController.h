#pragma once

#include <functional>
#include <string>

#include <esp_err.h>
#include <esp_http_server.h>

#include <cJSON.h>

#include "network/wifi.h"
#include "restApi/basicAuthenticator.h"
#include "interfaceUtils.h"

namespace sbc_pdu {
    namespace restApi {
        /**
         * WiFI manager REST API endpoints
         */
        class WifiController {
            public:
                /**
                 * Constructor
                 */
                explicit WifiController(Wifi *manager);
                
                /**
                 * Registers the endpoints
                 * @param server HTTP server handle
                 */
                void registerEndpoints(const httpd_handle_t &server);

                /**
                 * Scans the available WiFi APs
                 * @param request HTTP request
                 */
                static esp_err_t scan(httpd_req_t *request);
            private:
                /// Scan endpoint handler
                httpd_uri_t scanHandler;
        };
    }
}