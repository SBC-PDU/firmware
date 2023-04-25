/**
 * Copyright 2022-2023 Roman Ondráček
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "utils/interfaceUtils.h"

std::string InterfaceUtils::macToString(uint8_t buffer[6], const std::string &separator) {
    std::stringstream address;
    for (uint8_t i = 0; i < 6; ++i) {
        address << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]);
        if (i != 5) {
            address << separator;
        }
    }
    return address.str();
}