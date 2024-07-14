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
import { type AxiosResponse } from 'axios';

import { ApiClient } from '@/services/ApiClient';
import { type WifiAp, type WifiConfig } from '@/types/wifi';

/**
 * WiFi service
 */
export default class WifiService extends ApiClient {

	/**
	 * Returns WiFi configuration
	 * @return {Promise<WifiConfig>} WiFi configuration
	 */
	public async getConfig(): Promise<WifiConfig> {
		const response: AxiosResponse<WifiConfig> = await this.getClient().get('/wifi');
		return response.data;
	}

	/**
	 * Sets WiFi configuration
	 * @param {WifiConfig} config WiFi configuration
	 */
	public async setConfig(config: WifiConfig): Promise<void> {
		await this.getClient().put('/wifi', config);
	}

	/**
	 * Scans for WiFi access points
	 * @return {Promise<WifiAp[]>} WiFi access points
	 */
	public async scan(): Promise<WifiAp[]> {
		const response: AxiosResponse<WifiAp[]> = await this.getClient().get('/wifi/scan');
		return response.data;
	}

}
