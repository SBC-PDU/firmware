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
import { createRouter, createWebHistory, type NavigationGuardNext, type RouteLocationNormalized, type RouteRecordRaw } from 'vue-router';

import { useUserStore } from '@/store/user';

const routes: RouteRecordRaw[] = [
	{
		path: '/',
		component: () => import('@/layouts/default/Dashboard.vue'),
		children: [
			{
				path: ':pathMatch(.*)*',
				name: 'NotFound',
				component: () => import('@/views/NotFound.vue'),
			},
			{
				path: ':pathMatch(.*)',
				name: 'BadNotFound',
				component: () => import('@/views/NotFound.vue'),
			},
			{
				path: '',
				name: 'Home',
				component: () => import('@/views/Home.vue'),
			},
			{
				path: 'auth',
				children: [
					{
						path: 'sign',
						children: [
							{
								path: 'in',
								name: 'SignIn',
								component: () => import('@/views/auth/SignIn.vue'),
							},
						],
					},
				],
			},
			{
				path: 'config',
				children: [
					{
						path: 'auth',
						name: 'AuthConfig',
						component: () => import('@/views/config/AuthConfig.vue'),
					},
					{
						path: 'hostname',
						name: 'HostnameConfig',
						component: () => import('@/views/config/HostnameConfig.vue'),
					},
					{
						path: 'mqtt',
						name: 'MqttConfig',
						component: () => import('@/views/config/MqttConfig.vue'),
					},
					{
						path: 'ntp',
						name: 'NtpConfig',
						component: () => import('@/views/config/NtpConfig.vue'),
					},
					{
						path: 'wifi',
						name: 'WifiConfig',
						component: () => import('@/views/config/WifiConfig.vue'),
					},
				],
			},
			{
				path: 'system',
				children: [
					{
						path: 'info',
						name: 'SystemInfo',
						component: () => import('@/views/SystemInfo.vue'),
					},
				],
			},
		],
	},
];

const router = createRouter({
	history: createWebHistory(import.meta.env.BASE_URL),
	routes,
});

router.beforeEach((to: RouteLocationNormalized, _from: RouteLocationNormalized, next: NavigationGuardNext) => {
	const userStore = useUserStore();
	const whitelist = [
		'BadNotFound',
		'NotFound',
		'SignIn',
	];
	if (!userStore.isLoggedIn && !whitelist.includes(to.name as string)) {
		let query = { ...to.query };
		if (to.path !== '/' && to.name !== 'SignIn') {
			query = { ...query, redirect: to.path };
		}
		return next({ name: 'SignIn', query });
	}
	if (to.name === 'SignIn' && userStore.isLoggedIn) {
		return next((to.query.redirect as string | undefined) ?? '/');
	}
	next();
});

export default router;
