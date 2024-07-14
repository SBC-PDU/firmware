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
import child_process from 'node:child_process';
import path from 'node:path';
import { fileURLToPath, URL } from 'node:url';

import VueI18nPlugin from '@intlify/unplugin-vue-i18n/vite';
import { sentryVitePlugin } from '@sentry/vite-plugin';
import UnheadVite from '@unhead/addons/vite';
import vue from '@vitejs/plugin-vue';
import { defineConfig, loadEnv } from 'vite';
import { viteSingleFile } from 'vite-plugin-singlefile';
import VueDevTools from 'vite-plugin-vue-devtools';
import vuetify, { transformAssetUrls } from 'vite-plugin-vuetify';
import svgLoader from 'vite-svg-loader';

// Git commit hash
const gitCommitHash = child_process.execSync('git rev-parse --short HEAD').toString().trim();

// https://vitejs.dev/config/
export default defineConfig(({ command, mode }) => {
	const env = loadEnv(mode, process.cwd(), '');
	return {
		base: '/',
		plugins: [
			VueDevTools(),
			vue({
				template: { transformAssetUrls },
			}),
			// https://github.com/vuetifyjs/vuetify-loader/tree/master/packages/vite-plugin
			vuetify({
				autoImport: {
					labs: true,
				},
			}),
			UnheadVite(),
			VueI18nPlugin({
				include: [path.resolve(__dirname, 'src/locales/**')],
			}),
			svgLoader({ defaultImport: 'url' }),
			(env.VITE_SENTRY_ENABLED || process.env.SENTRY_ENABLED) === 'true' && sentryVitePlugin({
				release: {
					name: gitCommitHash,
				},
				url: env.VITE_SENTRY_URL || process.env.SENTRY_URL,
				org: env.VITE_SENTRY_ORG || process.env.SENTRY_ORG,
				project: env.VITE_SENTRY_PROJECT || process.env.SENTRY_PROJECT,
				authToken: env.VITE_SENTRY_AUTH_TOKEN || process.env.SENTRY_AUTH_TOKEN,
			}),
			...command === 'build' ? [
				viteSingleFile({
					removeViteModuleLoader: true,
				}),
			] : [],
		],
		define: {
			__GIT_COMMIT_HASH__: JSON.stringify(gitCommitHash),
		},
		optimizeDeps: {
			entries: [
				'src/components/**/*.{js,ts,vue}',
				'src/layouts/**/*.{js,ts,vue}',
				'src/pages/**/*.{js,ts,vue}',
				'src/App.vue',
				'src/main.ts',
			],
			exclude: ['vuetify'],
		},
		resolve: {
			alias: {
				'@': fileURLToPath(new URL('./src', import.meta.url)),
			},
			extensions: [
				'.js',
				'.json',
				'.jsx',
				'.mjs',
				'.ts',
				'.tsx',
				'.vue',
			],
		},
		server: {
			port: 3_000,
		},
	};
});
