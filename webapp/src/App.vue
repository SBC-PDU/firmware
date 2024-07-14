<!--
Copyright 2022-2024 Roman Ondráček <mail@romanondracek.cz>

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

<template :key='locale'>
	<v-theme-provider :theme='theme.global.name.value'>
		<v-app>
			<LoadingSpinner />
			<router-view />
		</v-app>
	</v-theme-provider>
</template>

<script lang='ts' setup>
import { storeToRefs } from 'pinia';
import { useHead } from 'unhead';
import { onBeforeMount, watch } from 'vue';
import { useI18n } from 'vue-i18n';
import { useTheme } from 'vuetify';

import LoadingSpinner from '@/components/layout/LoadingSpinner.vue';
import { useLocaleStore } from '@/store/locale';

const localeStore = useLocaleStore();
const { locale } = storeToRefs(localeStore);
const i18n = useI18n();
const theme = useTheme();

/**
 * Sets head options
 * @param {string} newLocale New locale
 */
function setHeadOptions(newLocale: string): void {
	useHead({
		htmlAttrs: {
			lang: newLocale,
		},
		titleTemplate: '%s %separator %siteName',
		templateParams: {
			siteName: i18n.t('core.title'),
			separator: '|',
		},
	});
}

onBeforeMount(() => {
	if (window.matchMedia('(prefers-color-scheme: dark)').matches) {
		theme.global.name.value = 'dark';
	} else {
		theme.global.name.value = 'light';
	}
	localeStore.setLocale(locale.value);
	setHeadOptions(locale.value);
});

watch(locale, setHeadOptions);
</script>
