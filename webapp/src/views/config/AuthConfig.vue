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
		<title>{{ $t('core.config.auth.title') }}</title>
	</Head>
	<Card>
		<template #title>
			{{ $t('core.config.auth.title') }}
		</template>
		<v-form ref='form' @submit.prevent='submit' v-if='config !== null'>
			<v-text-field
				v-model='config.username'
				:label='$t("core.config.auth.fields.username")'
				:rules='[
					(v: string|null) => FormValidator.isRequired(v, $t("core.config.auth.messages.emptyUsername")),
				]'
				required
				:prepend-inner-icon='mdiAccount'
			/>
			<PasswordField
				v-model='config.oldPassword'
				:label='$t("core.config.auth.fields.oldPassword")'
				:rules='[
					(v: string|null) => FormValidator.isRequired(v, $t("core.config.auth.messages.emptyOldPassword")),
				]'
				required
				:prepend-inner-icon='mdiKey'
			/>
			<PasswordField
				v-model='config.newPassword'
				:label='$t("core.config.auth.fields.newPassword")'
				:rules='[
					(v: string|null) => FormValidator.isRequired(v, $t("core.config.auth.messages.emptyNewPassword")),
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
import {mdiAccount, mdiKey, mdiPencil} from '@mdi/js';
import {Head} from '@vueuse/head';
import {Ref, ref} from 'vue';
import {useI18n} from 'vue-i18n';
import {toast} from 'vue3-toastify';
import {VForm} from 'vuetify/components';

import Card from '@/components/Card.vue';
import PasswordField from '@/components/PasswordField.vue';
import FormValidator from '@/helpers/formValidator';
import AuthenticationService from '@/services/AuthenticationService';
import {useLoadingSpinnerStore} from '@/store/loadingSpinner';
import {AuthConfig} from '@/types/auth';
import {useUserStore} from '@/store/user';
import {AxiosError} from 'axios';

const i18n = useI18n();
const loadingSpinner = useLoadingSpinnerStore();
const service = new AuthenticationService();
const userStore = useUserStore();

const config = ref<AuthConfig>({
	username: '',
	oldPassword: '',
	newPassword: '',
});
const form: Ref<typeof VForm | null> = ref(null);

/**
 * Loads auth configuration
 */
function loadData() {
	config.value.username = userStore.getName ?? '';
}

/**
 * Submits form
 */
async function submit(): Promise<void> {
	if (form.value === null) {
		return;
	}
	const {valid} = await form.value.validate();
	if (!valid) {
		return;
	}
	loadingSpinner.show();
	await service.setCredentials(config.value)
		.then(() => {
			userStore.setUserInfo({
				username: config.value.username,
				password: config.value.newPassword,
			});
			toast.success(i18n.t('core.config.auth.messages.success'));
		})
		.catch((error: AxiosError) => {
			if (error.response?.data === 'Incorrect current password.') {
				toast.error(i18n.t('core.config.auth.messages.incorrectOldPassword'));
				return;
			}
			toast.error(i18n.t('core.config.auth.messages.error'));
		})
		.finally(() => {
			loadingSpinner.hide();
		});
}

loadData();
</script>
