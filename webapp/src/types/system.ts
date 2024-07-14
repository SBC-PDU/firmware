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
 * Chip information
 */
export interface ChipInfo {
	/// Number of cores
	cores: number;
	/// Model
	model: string;
	/// Revision
	revision: number;
}

/**
 * IPv4 address configuration
 */
export interface Ipv4 {
	/// IPv4 address
	address: string;
	/// IPv4 netmask
	netmask: string;
	/// IPv4 gateway
	gateway: string;
}

/**
 * IPv6 address configuration
 */
export interface Ipv6 {
	/// IPv6 addresses
	addresses: string[];
}

/**
 * Network interface information
 */
export interface NetworkInterface {
	/// Interface name
	name: string;
	/// Interface MAC address
	macAddress: string;
	/// Interface IPv4 address configuration
	ipv4: Ipv4;
	/// Interface IPv6 address configuration
	ipv6: Ipv6;
	/// Interface DNS servers
	dns: string[];
	/// Interface hostname
	hostname: string;
	/// Is interface up?
	isUp: boolean;
}

/**
 * Heap information
 */
export interface HeapInfo {
	/// Total heap size
	total: number;
	/// Free heap size
	free: number;
}

/**
 * System information
 */
export interface SystemInfo {
	/// Chip information
	chip: ChipInfo;
	/// Network interfaces
	network: NetworkInterface[];
	/// Heap information
	heap: HeapInfo;
	/// Uptime
	uptime: number;
	/// ESP-IDF version
	idfVersion: string;
	/// Firmware version
	version: string;
}
