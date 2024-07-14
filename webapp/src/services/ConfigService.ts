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
import { type HostnameConfig, type MqttConfig, type NtpConfig } from '@/types/config';

/**
 * Config service
 */
export default class ConfigService extends ApiClient {

	/**
	 * Returns hostname config
	 * @return {Promise<HostnameConfig>} Hostname config
	 */
	public async getHostname(): Promise<HostnameConfig> {
		const response: AxiosResponse<HostnameConfig> = await this.getClient().get('/hostname');
		return response.data;
	}

	/**
	 * Sets hostname config
	 * @param {HostnameConfig} hostname Hostname config
	 */
	public async setHostname(hostname: HostnameConfig): Promise<void> {
		await this.getClient().put('/hostname', hostname);
	}

	/**
	 * Returns MQTT config
	 * @return {Promise<MqttConfig>} MQTT config
	 */
	public async getMqtt(): Promise<MqttConfig> {
		const response: AxiosResponse<MqttConfig> = await this.getClient().get('/mqtt');
		return response.data;
	}

	/**
	 * Sets MQTT config
	 * @param {MqttConfig} mqtt MQTT config
	 */
	public async setMqtt(mqtt: MqttConfig): Promise<void> {
		await this.getClient().put('/mqtt', mqtt);
	}

	/**
	 * Returns NTP config
	 * @return {Promise<NtpConfig>} NTP config
	 */
	public async getNtp(): Promise<NtpConfig> {
		const response: AxiosResponse<NtpConfig> = await this.getClient().get('/ntp');
		return response.data;
	}

	/**
	 * Sets NTP config
	 * @param {NtpConfig} ntp NTP config
	 */
	public async setNtp(ntp: NtpConfig): Promise<void> {
		await this.getClient().put('/ntp', ntp);
	}

}
