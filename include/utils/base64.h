/**
 * Copyright 2022-2024 Roman Ondráček <mail@romanondracek.cz>
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
 * Base64
 */
class Base64 {
	public:
		/**
		 * Encodes string into base64
		 * @param value String to encode
		 * @return Base64 enccoded string
		 */
		static const std::string encode(const std::string &value) {
			size_t n = 0;
			size_t out = 0;
			const unsigned char* bufferToEncode = reinterpret_cast<const unsigned char*>(value.c_str());
			// Calculate buffer size
			mbedtls_base64_encode(nullptr, 0, &n, bufferToEncode, value.length());
			char *buffer = new char[n + 1];
			if (buffer) {
				// Encode base64 from string
				mbedtls_base64_encode(reinterpret_cast<unsigned char*>(buffer), n, &out, bufferToEncode, value.length());
			} else {
				return "";
			}
			std::string encodedValue = std::string(buffer);
			delete[] buffer;
			return encodedValue;
		}
};
