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
import {Address6} from 'ip-address';
import {isIPv6} from 'is-ip';

import {ApiClient} from '@/services/ApiClient';
import {SystemInfo} from '@/types/system';

/**
 * System service
 */
export default class SystemService extends ApiClient {

	/**
	 * Returns system info
	 * @returns {Promise<SystemInfo>} System info
	 */
	public getInfo(): Promise<SystemInfo> {
		return this.getClient().get('system/info')
			.then((response: AxiosResponse) => {
				const info = response.data as SystemInfo;
				for (const [ifaceIndex, iface] of info.network.entries()) {
					for (const [ipv6Index, ipv6] of iface.ipv6.addresses.entries()) {
						info.network[ifaceIndex].ipv6.addresses[ipv6Index] = (new Address6(ipv6)).correctForm();
					}
					for (const [dnsIndex, dns] of iface.dns.entries()) {
						if (!isIPv6(dns)) {
							continue;
						}
						info.network[ifaceIndex].dns[dnsIndex] = (new Address6(dns)).correctForm();
					}
				}
				return info;
			});
	}

}
