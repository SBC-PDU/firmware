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
import {
	setUser as setSentryUser,
	type User as SentryUser,
} from '@sentry/vue';
import { type AxiosBasicCredentials } from 'axios';
import { defineStore } from 'pinia';

import router from '@/router';
import AuthenticationService from '@/services/AuthenticationService';
import { type Credentials } from '@/types/auth';

/**
 * User store state
 */
interface UserState {
	/// User name
	username: string | null;
	/// User password
	password: string | null;
}

export const useUserStore = defineStore('user', {
	state: (): UserState => ({
		password: null,
		username: null,
	}),
	actions: {
		/**
		 * Sets credentials
		 * @param {Credentials} credentials Credentials
		 */
		setUserInfo(credentials: Credentials): void {
			this.password = credentials.password;
			this.username = credentials.username;
			const sentryUser: SentryUser = {
				username: this.username,
				ip_address: '{{auto}}',
			};
			setSentryUser(sentryUser);
		},
		/**
		 * Sign in
		 * @param {Credentials} credentials User credentials
		 */
		async signIn(credentials: Credentials): Promise<void> {
			const service = new AuthenticationService();
			await service.verify(credentials);
			this.setUserInfo(credentials);
		},
		/**
		 * Sign out
		 */
		async signOut(): Promise<void> {
			this.password = null;
			this.username = null;
			setSentryUser(null);
			await router.push('/auth/sign/in');
		},
	},
	getters: {
		/**
		 * Checks if user is logged in
		 * @param {UserState} state User state
		 * @return {boolean} True if user is logged in
		 */
		isLoggedIn(state: UserState): boolean {
			return !(state.username === null || state.password === null);

		},
		/**
		 * Returns basic auth credentials
		 * @param {UserState} state User state
		 * @return {AxiosBasicCredentials|null} Basic auth credentials
		 */
		credentials(state: UserState): AxiosBasicCredentials | null {
			if (state.username === null || state.password === null) {
				return null;
			}
			return {
				username: state.username,
				password: state.password,
			};
		},
		/**
		 * Returns user email
		 * @param {UserState} state User state
		 * @return {string|null} User email
		 */
		getName(state: UserState): string | null {
			return state.username;
		},
	},
	persist: true,
});
