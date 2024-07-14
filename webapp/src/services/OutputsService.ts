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
import { type Output } from '@/types/outputs';

/**
 * Power outputs service
 */
export default class OutputsService extends ApiClient {

	/**
	 * Returns current outputs state
	 * @return {Promise<Output[]>} Outputs state
	 */
	public async get(): Promise<Output[]> {
		const response: AxiosResponse<Output[]> = await this.getClient().get('/outputs');
		return response.data;
	}

	/**
	 * Switches output
	 * @param {number} index Output index
	 * @param {boolean} state Output state
	 */
	public async switch(index: number, state: boolean): Promise<void> {
		await this.getClient().post('/outputs/switch', {
			output: index,
			state: state,
		});
	}

}
