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
#pragma once

#include <string>

#include <mbedtls/base64.h>

/**
 * Binary coded decimal (BCD) utils
 */
class Bcd {
	public:
		/**
		 * Converts BCD to binary number
		 * @param bcd BCD number
		 * @return Binary number
		 */
		static inline uint16_t bcd2bin(uint8_t bcd) {
			return ((bcd) & 0x0f) + ((bcd) >> 4) * 10;
		}

		/**
		 * Converts binary number to BCD
		 * @param bin Binary number
		 * @return BCD number
		 */
		static inline uint8_t bin2bcd(uint16_t bin) {
			return (((bin) / 10) << 4) + (bin) % 10;
		}
};
