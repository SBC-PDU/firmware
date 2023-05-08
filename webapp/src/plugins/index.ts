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
import type {App} from 'vue';

import head from '@/plugins/head';
import i18n from '@/plugins/i18n';
import toastify, {ToastifyOptions} from '@/plugins/toastify';
import vuetify from '@/plugins/vuetify';
import pinia from '@/store';
import router from '@/router';
import registerSentry from '@/plugins/sentry';

export function registerPlugins(app: App) {
	app
		.use(pinia)
		.use(router)
		.use(i18n)
		.use(head)
		.use(toastify, ToastifyOptions)
		.use(vuetify);
	registerSentry(app, router);
}
