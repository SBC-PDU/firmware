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
		<title>{{ $t('core.system.info.title') }}</title>
	</Head>
	<Card>
		<template #title>
			{{ $t('core.system.info.title') }}
		</template>
		<v-table v-if='info !== null'>
			<tbody>
				<tr>
					<th colspan='2' style='background-color: #f5f5f5;'>
						{{ $t('core.system.info.chip.title') }}
					</th>
				</tr>
				<tr>
					<th>{{ $t('core.system.info.chip.model') }}</th>
					<td>{{ info.chip.model }}</td>
				</tr>
				<tr>
					<th>{{ $t('core.system.info.chip.revision') }}</th>
					<td>{{ info.chip.revision }}</td>
				</tr>
				<template v-for='networkIface in info.network' :key='networkIface.name'>
					<tr>
						<th colspan='2' style='background-color: #f5f5f5;'>
							{{ $t('core.system.info.network.title', {name: networkIface.name}) }}
						</th>
					</tr>
					<tr>
						<th>{{ $t('core.system.info.network.macAddress') }}</th>
						<td>{{ networkIface.macAddress }}</td>
					</tr>
					<tr>
						<th>{{ $t('core.system.info.network.ipv4.title') }}</th>
						<td>
							<strong>{{ $t('core.system.info.network.ipv4.address') }}: </strong>
							{{ networkIface.ipv4.address }}
							<br>
							<strong>{{ $t('core.system.info.network.ipv4.netmask') }}: </strong>
							{{ networkIface.ipv4.netmask }}
							<br>
							<strong>{{ $t('core.system.info.network.ipv4.gateway') }}: </strong>
							{{ networkIface.ipv4.gateway }}
							<br>
						</td>
					</tr>
					<tr>
						<th>{{ $t('core.system.info.network.ipv6.title') }}</th>
						<td>
							<ul>
								<li v-for='address in networkIface.ipv6.addresses' :key='address'>{{ address }}</li>
							</ul>
						</td>
					</tr>
					<tr>
						<th>{{ $t('core.system.info.network.dns.title') }}</th>
						<td>
							<ul>
								<li v-for='address in networkIface.dns' :key='address'>{{ address }}</li>
							</ul>
						</td>
					</tr>
					<tr>
						<th>{{ $t('core.system.info.network.hostname') }}</th>
						<td>{{ networkIface.hostname }}</td>
					</tr>
					<tr>
						<th>{{ $t('core.system.info.network.isUp') }}</th>
						<td>
							<v-icon
								:icon='networkIface.isUp ? mdiCheck : mdiClose'
								:color='networkIface.isUp ? "success" : "error"'
							/>
						</td>
					</tr>
				</template>
				<tr>
					<th colspan='2' style='background-color: #f5f5f5;'>
						{{ $t('core.system.info.heap.title') }}
					</th>
				</tr>
				<tr>
					<th>{{ $t('core.system.info.heap.total') }}</th>
					<td>{{ convertBytesToReadableFormat(info.heap.total) }}</td>
				</tr>
				<tr>
					<th>{{ $t('core.system.info.heap.free') }}</th>
					<td>{{ convertBytesToReadableFormat(info.heap.free) }}</td>
				</tr>
				<tr>
					<th colspan='2' style='background-color: #f5f5f5;'>
						{{ $t('core.system.info.firmware.title') }}
					</th>
				</tr>
				<tr>
					<th>{{ $t('core.system.info.firmware.version') }}</th>
					<td>{{ info.version }}</td>
				</tr>
				<tr>
					<th>{{ $t('core.system.info.firmware.idfVersion') }}</th>
					<td>{{ info.idfVersion }}</td>
				</tr>
				<tr>
					<th>{{ $t('core.system.info.uptime') }}</th>
					<td>{{ formatUptime(info.uptime) }}</td>
				</tr>
			</tbody>
		</v-table>
	</Card>
</template>

<script lang='ts' setup>
import {mdiCheck, mdiClose} from '@mdi/js';
import {Head} from '@vueuse/head';
import {ref} from 'vue';

import Card from '@/components/Card.vue';
import {useLoadingSpinnerStore} from '@/store/loadingSpinner';
import SystemService from '@/services/SystemService';
import {SystemInfo} from '@/types/system';

const loadingSpinner = useLoadingSpinnerStore();
const service = new SystemService();

const info = ref<SystemInfo|null>(null);

/**
 * Loads system information
 */
function loadSystemInfo() {
	loadingSpinner.show();
	service.getInfo().then((response: SystemInfo) => {
		info.value = response;
	}).finally(() => {
		loadingSpinner.hide();
	});
}

loadSystemInfo();

/**
 * Converts bytes to a readable format
 * @param {number} bytes The bytes to convert
 * @returns {string} Bytes in a readable format
 */
function convertBytesToReadableFormat(bytes: number): string {
	const sizes = ['B', 'kB', 'MB', 'GB', 'TB'];
	if (bytes === 0) {
		return '0 B';
	}
	const i = Math.floor(Math.log(bytes) / Math.log(1024));
	return Math.round(bytes / Math.pow(1024, i)) + ' ' + sizes[i];
}

/**
 * Formats the uptime
 * @param {number} uptime The uptime in seconds
 * @returns {string} The formatted uptime
 */
function formatUptime(uptime: number): string {
	const days = Math.floor(uptime / 86400);
	const hours = Math.floor((uptime % 86400) / 3600);
	const minutes = Math.floor(((uptime % 86400) % 3600) / 60);
	const seconds = Math.floor(((uptime % 86400) % 3600) % 60);
	return `${days}d ${hours}h ${minutes}m ${seconds}s`;
}
</script>
