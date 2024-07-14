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

#include "nvsManager.h"

/**
 * Hostname manager
 */
class HostnameManager {
	public:
		/**
		 * Returns the hostname
		 * @return Hostname
		 */
		std::string get();

		/**
		 * Sets the hostname
		 * @param hostname Hostname to set
		 */
		void set(const std::string &hostname);

	private:
		/// NVS manager
		NvsManager nvs = NvsManager("hostname");
};
