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
import {Output} from '@/types/outputs';

/**
 * Power outputs service
 */
export default class OutputsService extends ApiClient {

	/**
	 * Returns current outputs state
	 * @returns {Promise<Output[]>} Outputs state
	 */
	public get(): Promise<Output[]> {
		return this.getClient().get('outputs')
			.then((response: AxiosResponse) => response.data as Output[]);
	}

	/**
	 * Switches output
	 * @param {number} index Output index
	 * @param {boolean} state Output state
	 */
	public switch(index: number, state: boolean): Promise<AxiosResponse> {
		return this.getClient().post('outputs/switch', {
			output: index,
			state: state,
		});
	}

}
