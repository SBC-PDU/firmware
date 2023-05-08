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
import {HostnameConfig, MqttConfig, NtpConfig} from '@/types/config';

/**
 * Config service
 */
export default class ConfigService extends ApiClient {

	/**
	 * Returns hostname config
	 * @returns {Promise<HostnameConfig>} Hostname config
	 */
	public getHostname(): Promise<HostnameConfig> {
		return this.getClient().get('hostname')
			.then((response: AxiosResponse) => response.data as HostnameConfig);
	}

	/**
	 * Sets hostname config
	 * @param {HostnameConfig} hostname Hostname config
	 */
	public setHostname(hostname: HostnameConfig): Promise<AxiosResponse> {
		return this.getClient().put('hostname', hostname);
	}

	/**
	 * Returns MQTT config
	 * @returns {Promise<MqttConfig>} MQTT config
	 */
	public getMqtt(): Promise<MqttConfig> {
		return this.getClient().get('mqtt')
			.then((response: AxiosResponse) => response.data as MqttConfig);
	}

	/**
	 * Sets MQTT config
	 * @param {MqttConfig} mqtt MQTT config
	 */
	public setMqtt(mqtt: MqttConfig): Promise<AxiosResponse> {
		return this.getClient().put('mqtt', mqtt);
	}

	/**
	 * Returns NTP config
	 * @returns {Promise<NtpConfig>} NTP config
	 */
	public getNtp(): Promise<NtpConfig> {
		return this.getClient().get('ntp')
			.then((response: AxiosResponse) => response.data as NtpConfig);
	}

	/**
	 * Sets NTP config
	 * @param {NtpConfig} ntp NTP config
	 */
	public setNtp(ntp: NtpConfig): Promise<AxiosResponse> {
		return this.getClient().put('ntp', ntp);
	}

}
