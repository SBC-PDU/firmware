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
	<v-navigation-drawer
		v-model='isVisible'
		color='primary'
		:rail='isMinimized'
	>
		<SidebarItems :items='items()'/>
		<template #append>
			<v-list>
				<v-list-item density='compact' style='margin-top: auto;' @click.stop='sidebarStore.toggleSize()'>
					<v-list-item-action>
						<v-icon :icon='isMinimized ? mdiChevronRight : mdiChevronLeft' />
					</v-list-item-action>
				</v-list-item>
			</v-list>
		</template>
	</v-navigation-drawer>
</template>

<script lang='ts' setup>
import {mdiChevronLeft, mdiChevronRight, mdiCog, mdiInformationOutline, mdiLogin, mdiViewDashboard} from '@mdi/js';
import {storeToRefs} from 'pinia';
import {useI18n} from 'vue-i18n';
import {useDisplay} from 'vuetify';

import SidebarItems from '@/components/layout/sidebar/SidebarItems.vue';
import {useSidebarStore} from '@/store/sidebar';
import {useUserStore} from '@/store/user';
import {SidebarLink} from '@/types/sidebar';

const i18n = useI18n();
const display = useDisplay();
const sidebarStore = useSidebarStore();
const userStore = useUserStore();
const {isLoggedIn} = storeToRefs(userStore);

const {isMinimized, isVisible} = storeToRefs(sidebarStore);

sidebarStore.setVisibility(display.xlAndUp.value);

/**
 * Returns sidebar items
 * @returns {SidebarLink[]} Sidebar items
 */
function items(): SidebarLink[] {
	let links: SidebarLink[];
	if (isLoggedIn.value) {
		links = [
			{
				icon: mdiViewDashboard,
				title: i18n.t('core.dashboard.title').toString(),
				to: '/',
			},
			{
				icon: mdiInformationOutline,
				title: i18n.t('core.system.info.title').toString(),
				to: '/system/info',
			},

			{
				icon: mdiCog,
				title: i18n.t('core.config.title').toString(),
				children: [
					{
						title: i18n.t('core.config.auth.sidebar').toString(),
						to: '/config/auth',
					},
					{
						title: i18n.t('core.config.hostname.sidebar').toString(),
						to: '/config/hostname',
					},
					{
						title: i18n.t('core.config.mqtt.sidebar').toString(),
						to: '/config/mqtt',
					},
					{
						title: i18n.t('core.config.ntp.sidebar').toString(),
						to: '/config/ntp',
					},
					{
						title: i18n.t('core.config.wifi.sidebar').toString(),
						to: '/config/wifi',
					},
				],
			},
		];
	} else {
		links = [
			{
				icon: mdiLogin,
				title: i18n.t('core.sign.in.title').toString(),
				to: '/auth/sign/in',
			},
		];
	}
	return links;
}

</script>
