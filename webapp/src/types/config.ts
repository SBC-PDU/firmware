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

/**
 * Hostname configuration
 */
export interface HostnameConfig {
	/// Hostname
	hostname: string;
}

/**
 * MQTT configuration
 */
export interface MqttConfig {
	/// MQTT broker URI
	uri: string;
	/// MQTT broker username
	username: string;
	/// MQTT broker password
	password: string;
}

/**
 * NTP timezone
 */
export enum NtpTimezone {
	ACST = 'ACST',
	ACST_ACDT = 'ACST/ACDT',
	AEST = 'AEST',
	AEST_AEDT = 'AEST/AEDT',
	AKST_AKDT = 'AKST/AKDT',
	AST_ADT = 'AST/ADT',
	AWST_AWDT = 'AWST/AWDT',
	BRST_BRDT = 'BRST/BRDT',
	CET_CEST = 'CET/CEST',
	CST = 'CST',
	CST_CDT = 'CST/CDT',
	EET_EEST = 'EET/EEST',
	EST_EDT = 'EST/EDT',
	GMT_BST = 'GMT/BST',
	GMT_IST = 'GMT/IST',
	HAW = 'HAW',
	HKT = 'HKT',
	MSK_MSD = 'MSK/MSD',
	RMST_RMDT = 'RMST/RMDT',
	MST = 'MST',
	MST_MDT = 'MST/MDT',
	NST_NDT = 'NST/NDT',
	NZST_NZDT = 'NZST/NZDT',
	PST_PDT = 'PST/PDT',
	SGT = 'SGT',
	ULAT_ULAST = 'ULAT/ULAST',
	WET_WEST = 'WET/WEST',
	WIB = 'WIB',
}

/**
 * NTP configuration
 */
export interface NtpConfig {
	/// NTP servers
	servers: string[];
	/// NTP timezone
	timezone: NtpTimezone;
}
