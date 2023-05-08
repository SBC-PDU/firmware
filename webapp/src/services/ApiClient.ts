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
import axios, {AxiosError, AxiosInstance, AxiosResponse} from 'axios';
import {storeToRefs} from 'pinia';

import router from '@/router';
import {useUserStore} from '@/store/user';

/**
 * REST API client
 */
export class ApiClient {

	/**
	 * @property {AxiosInstance} client Axios instance
	 */
	protected client: AxiosInstance;

	/**
	 * Constructor
	 */
	constructor() {
		const userStore = useUserStore();
		const {credentials} = storeToRefs(userStore);
		this.client = axios.create({
			baseURL: import.meta.env.VITE_API_BASE_URL,
		});
		// Register request interceptor for adding Authorization header
		this.client.interceptors.request.use((config) => {
			// Add Authorization header if credentials is present
			if (credentials.value !== null) {
				config.auth = credentials.value;
			}
			return config;
		});
		// Register response interceptor for handling HTTP errors
		this.client.interceptors.response.use(
			(response: AxiosResponse): AxiosResponse => (response),
			(error: AxiosError) => {
				console.error(error);
				// Handle network error
				if (error.response === undefined) {
					return Promise.reject(error);
				}
				// Handle HTTP Error 401 Unauthorized response
				if (error.response.status === 401) {
					const userStore = useUserStore();
					userStore.signOut();
					// Prevent duplicate redirect to sign in page
					if (router.currentRoute.value.name !== 'SignIn') {
						router.push({name: 'SignIn', query: {redirect: router.currentRoute.value.path}});
					}
				}
				// Handle other HTTP errors
				return Promise.reject(error);
			}
		);
	}

	/**
	 * Returns Axios instance
	 * @returns {AxiosInstance} Axios instance
	 */
	public getClient(): AxiosInstance {
		return this.client;
	}

}
