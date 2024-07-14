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

/**
 * Wifi authentication mode
 */
export enum WifiAuthMode {
	/// Open
	OPEN = 'Open',
	/// WEP
	WEP = 'WEP',
	/// WPA Personal
	WPA_Personal = 'WPA-Personal',
	/// WPA2 Personal
	WPA2_Personal = 'WPA2-Personal',
	/// WPA3 Personal
	WPA3_Personal = 'WPA3-Personal',
	/// WPA/WPA2 Personal
	WPA_WPA2_Personal = 'WPA/WPA2-Personal',
	/// WPA2/WPA3 Personal
	WPA2_WPA3_Personal = 'WPA2/WPA3-Personal',
}

/**
 * Wifi configuration
 */
export interface WifiConfig {
	/// Authentication mode
	authMode: WifiAuthMode;
	/// SSID
	ssid: string;
	/// Pre-shared key
	psk: string;
}

/**
 * WiFi access point
 */
export interface WifiAp {
	/// Authentication mode
	authMode: WifiAuthMode;
	/// BSSID
	bssid: string;
	/// Is in use?
	inUse: boolean;
	/// Signal strength
	rssi: number;
	/// SSID
	ssid: string;
}
