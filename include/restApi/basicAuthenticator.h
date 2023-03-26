#pragma once

#include <functional>
#include <string>

#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_tls_crypto.h>

#include <cJSON.h>

#include "nvs_manager.h"

namespace sbc_pdu {
    namespace restApi {
        /**
         * @brief HTTP Basic authenticator
         */
        class BasicAuthenticator {
            public:
                /**
                 * @brief Constructor
                 */
                explicit BasicAuthenticator();

                /**
                 * @brief Returns Expected HTTP Authorization header
                 * 
                 * @return std::string Expected HTTP Authorization header
                 */
                std::string getExpectedAuthorizationHeader();

                /**
                 * @brief Authenticate the user
                 * 
                 * @param request HTTP request
                 * @return bool Authentication status
                 */
                bool authenticate(httpd_req_t *request);
                
                /**
                 * @brief Creates Unauthorized HTTP Response
                 * 
                 * @param request HTTP request
                 */
                void createUnauthorizedResponse(httpd_req_t *request);
            private:
                /// @brief Expected Authorizaton HTTP header
                std::string expectedAuthorizationHeader;
                /// @brief  Username
                std::string username;
                /// @brief Password
                std::string password;
                /// NVS manager
                NvsManager nvs = NvsManager("httpCredentials");
        };
    }
}
