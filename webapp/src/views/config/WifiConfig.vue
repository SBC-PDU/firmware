<!--
Copyright 2022-2023 Roman Ondráček

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->

<template>
	<Head>
		<title>{{ $t('core.config.wifi.title') }}</title>
	</Head>
	<Card>
		<template #title>
			{{ $t('core.config.wifi.title') }}
		</template>
		<v-form ref='form' @submit.prevent='submit' v-if='config !== null'>
			<v-select
				v-model='config.authMode'
				:label='$t("core.config.wifi.fields.authMode")'
				:items='Object.values(WifiAuthMode)'
				:rules='[
					(v: string|null) => FormValidator.isRequired(v, $t("core.config.wifi.messages.emptyAuthMode")),
				]'
				required
				:prepend-inner-icon='mdiShieldKey'
			/>
			<v-text-field
				v-model='config.ssid'
				:label='$t("core.config.wifi.fields.ssid")'
				:rules='[
					(v: string|null) => FormValidator.isRequired(v, $t("core.config.wifi.messages.emptySsid")),
				]'
				required
				:prepend-inner-icon='mdiWifi'
			/>
			<PasswordField
				v-if='config.authMode !== WifiAuthMode.OPEN'
				v-model='config.psk'
				:label='$t("core.config.wifi.fields.psk")'
				:rules='[
					(v: string|null) => FormValidator.isRequired(v, $t("core.config.wifi.messages.emptyPsk")),
				]'
				required
				:prepend-inner-icon='mdiKey'
			/>
			<v-btn
				color='primary'
				type='submit'
				:prepend-icon='mdiPencil'
			>
				{{ $t('core.actions.edit') }}
			</v-btn>
		</v-form>
	</Card>
</template>

<script lang='ts' setup>
import {mdiKey, mdiPencil, mdiShieldKey, mdiWifi} from '@mdi/js';
import {Head} from '@vueuse/head';
import {Ref, ref} from 'vue';
import {useI18n} from 'vue-i18n';
import {toast} from 'vue3-toastify';
import {VForm} from 'vuetify/components';

import Card from '@/components/Card.vue';
import PasswordField from '@/components/PasswordField.vue';
import FormValidator from '@/helpers/formValidator';
import WifiService from '@/services/WifiService';
import {useLoadingSpinnerStore} from '@/store/loadingSpinner';
import {WifiAuthMode, WifiConfig} from '@/types/wifi';

const i18n = useI18n();
const loadingSpinner = useLoadingSpinnerStore();
const service = new WifiService();

const config = ref<WifiConfig|null>(null);
const form: Ref<typeof VForm | null> = ref(null);

/**
 * Loads WiFi configuration
 */
function loadData() {
	loadingSpinner.show();
	service.getConfig().then((response: WifiConfig) => {
		config.value = response;
	}).finally(() => {
		loadingSpinner.hide();
	});
}

/**
 * Submits form
 */
async function submit(): Promise<void> {
	if (form.value === null) {
		return;
	}
	const {valid} = await form.value.validate();
	if (!valid || config.value === null) {
		return;
	}
	loadingSpinner.show();
	await service.setConfig(config.value as WifiConfig)
		.then(() => {
			toast.success(i18n.t('core.config.wifi.messages.success'));
		})
		.catch(() => {
			toast.error(i18n.t('core.config.wifi.messages.error'));
		})
		.finally(() => {
			loadingSpinner.hide();
		});
}

loadData();
</script>
