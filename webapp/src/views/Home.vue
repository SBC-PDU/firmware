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
		<title>{{ $t('core.dashboard.title') }}</title>
	</Head>
	<v-toolbar color='primary' density='compact'>
		<v-toolbar-title>
			{{ $t('core.dashboard.title') }}
		</v-toolbar-title>
		<v-toolbar-items>
			<v-btn
				color='grey'
				:prepend-icon='display.smAndUp.value ? mdiRefresh : undefined'
				variant='elevated'
				@click='loadData()'
			>
				<span v-if='display.smAndUp.value'>
					{{ $t('core.dashboard.outputs.reload') }}
				</span>
				<v-icon v-else :icon='mdiRefresh' />
			</v-btn>
		</v-toolbar-items>
	</v-toolbar>
	<Card>
		<v-table>
			<thead>
				<tr>
					<th>{{ $t('core.dashboard.outputs.fields.index') }}</th>
					<th>{{ $t('core.dashboard.outputs.fields.state') }}</th>
					<th>{{ $t('core.dashboard.outputs.fields.alert') }}</th>
					<th>{{ $t('core.dashboard.outputs.fields.voltage') }}</th>
					<th>{{ $t('core.dashboard.outputs.fields.current') }}</th>
					<th>{{ $t('core.dashboard.outputs.fields.power') }}</th>
				</tr>
			</thead>
			<tbody>
				<tr v-for='output in outputs' :key='output.index'>
					<td>{{ output.index }}</td>
					<td>
						<v-switch
							v-model='output.enabled'
							color='primary'
							class='my-auto'
							@update:model-value='switchOutput(output)'
						/>
					</td>
					<td>
						<v-chip :color='output.alert ? "error" : "success"'>
							{{ $t(`core.dashboard.outputs.fields.alertValues.${output.alert.toString()}`) }}
						</v-chip>
					</td>
					<td>{{ Number(output.voltage).toFixed(2).toString() }} V</td>
					<td>{{ Number(output.current).toFixed(2).toString() }} mA</td>
					<td>{{ Number(output.voltage * output.current).toFixed(2).toString() }} mW</td>
				</tr>
			</tbody>
		</v-table>
	</Card>
</template>

<script lang='ts' setup>
import {mdiRefresh} from '@mdi/js';
import {Head} from '@vueuse/head';
import {ref} from 'vue';
import {useI18n} from 'vue-i18n';
import {useDisplay} from 'vuetify';
import {toast} from 'vue3-toastify';

import Card from '@/components/Card.vue';
import {useLoadingSpinnerStore} from '@/store/loadingSpinner';
import OutputsService from '@/services/OutputsService';
import {Output} from '@/types/outputs';

const display = useDisplay();
const i18n = useI18n();
const loadingSpinner = useLoadingSpinnerStore();
const service = new OutputsService();

const outputs = ref<Output[]>([]);

/**
 * Load data from API
 */
function loadData() {
	loadingSpinner.show();
	service.get()
		.then((response: Output[]) => {
			outputs.value = response;
		})
		.catch(() => {
			toast.error(i18n.t('core.dashboard.outputs.messages.error.load').toString());
		})
		.finally(() => {
			loadingSpinner.hide();
		});
}

/**
 * Switches output
 * @param {Output} output Output to switch
 */
function switchOutput(output: Output): Promise<void> {
	loadingSpinner.show();
	return service.switch(output.index, output.enabled)
		.then(() => {
			if (output.enabled) {
				toast.success(i18n.t('core.dashboard.outputs.messages.success.switchedOn', {
					index: output.index
				}).toString());
			} else {
				toast.success(i18n.t('core.dashboard.outputs.messages.success.switchedOff', {
					index: output.index
				}).toString());
			}
		})
		.catch(() => {
			if (output.enabled) {
				toast.error(i18n.t('core.dashboard.outputs.messages.error.switchedOn', {
					index: output.index
				}).toString());
			} else {
				toast.error(i18n.t('core.dashboard.outputs.messages.error.switchedOff', {
					index: output.index
				}).toString());
			}
		})
		.finally(() => {
			loadData();
			loadingSpinner.hide();
		});
}

loadData();
</script>
