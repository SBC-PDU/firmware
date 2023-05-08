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
		<title>{{ $t('core.config.hostname.title') }}</title>
	</Head>
	<Card>
		<template #title>
			{{ $t('core.config.hostname.title') }}
		</template>
		<v-form ref='form' @submit.prevent='submit' v-if='config !== null'>
			<v-text-field
				v-model='config.hostname'
				:label='$t("core.config.hostname.fields.hostname")'
				:rules='[
					(v: string|null) => FormValidator.isRequired(v, $t("core.config.hostname.messages.emptyHostname")),
				]'
				required
				:prepend-inner-icon='mdiServerNetwork'
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
import {mdiPencil, mdiServerNetwork} from '@mdi/js';
import {Head} from '@vueuse/head';
import {Ref, ref} from 'vue';
import {useI18n} from 'vue-i18n';
import {toast} from 'vue3-toastify';
import {VForm} from 'vuetify/components';

import Card from '@/components/Card.vue';
import FormValidator from '@/helpers/formValidator';
import ConfigService from '@/services/ConfigService';
import {useLoadingSpinnerStore} from '@/store/loadingSpinner';
import {HostnameConfig} from '@/types/config';

const i18n = useI18n();
const loadingSpinner = useLoadingSpinnerStore();
const service = new ConfigService();

const config = ref<HostnameConfig|null>(null);
const form: Ref<typeof VForm | null> = ref(null);

/**
 * Loads Hostname configuration
 */
function loadData() {
	loadingSpinner.show();
	service.getHostname().then((response: HostnameConfig) => {
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
	await service.setHostname(config.value as HostnameConfig)
		.then(() => {
			toast.success(i18n.t('core.config.hostname.messages.success'));
		})
		.catch(() => {
			toast.error(i18n.t('core.config.hostname.messages.error'));
		})
		.finally(() => {
			loadingSpinner.hide();
		});
}

loadData();
</script>
