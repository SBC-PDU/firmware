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
import {AxiosResponse} from 'axios';

import {ApiClient} from '@/services/ApiClient';
import {WifiAp, WifiConfig} from '@/types/wifi';

/**
 * WiFi service
 */
export default class WifiService extends ApiClient {

	/**
	 * Returns WiFi configuration
	 * @returns {Promise<WifiConfig>} WiFi configuration
	 */
	public getConfig(): Promise<WifiConfig> {
		return this.getClient().get('wifi')
			.then((response: AxiosResponse) => response.data as WifiConfig);
	}

	/**
	 * Sets WiFi configuration
	 * @param {WifiConfig} config WiFi configuration
	 */
	public setConfig(config: WifiConfig): Promise<AxiosResponse> {
		return this.getClient().put('wifi', config);
	}

	/**
	 * Scans for WiFi access points
	 * @returns {Promise<WifiAp[]>} WiFi access points
	 */
	public scan(): Promise<WifiAp[]> {
		return this.getClient().get('wifi/scan')
			.then((response: AxiosResponse) => response.data as WifiAp[]);
	}

}
