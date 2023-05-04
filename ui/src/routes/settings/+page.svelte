<script>
	import { variables } from '$lib/utils/variables';
	import { onMount } from 'svelte';
	import { Button, FormGroup, Input, Label } from 'sveltestrap';
	import Fa from 'svelte-fa/src/fa.svelte';
	import { faFloppyDisk } from '@fortawesome/pro-solid-svg-icons/faFloppyDisk';
	import { toast } from '@zerodevx/svelte-toast';

	let config = {};

	onMount(async () => {
		const response = await fetch(`/api/config`, {
			headers: { 'Content-type': 'application/json' }
		}).catch((error) => console.log(error));
		if (response.ok) config = await response.json();
		else {
			toast.push(`Error ${response.status} ${response.statusText}<br>Unable to receive current settings.`, variables.toast.error);
		}
	});

	async function doSaveSettings() {
		fetch(`/api/config`, {
			method: 'POST',
			body: JSON.stringify(config),
			headers: { 'Content-type': 'application/json' }
		})
			.then((response) => {
				if (response.ok) {
					toast.push(`Settings successfully saved`, variables.toast.success);
				} else {
					toast.push(`Error ${response.status} ${response.statusText}<br>Unable to store new AP configuration.`, variables.toast.error);
				}
			})
			.catch((error) => console.log(error));
	}
</script>

<svelte:head>
	<title>Sensor Settings</title>
</svelte:head>

<h4>Configuration</h4>
<FormGroup>
	<Label for="hostname">Hostname</Label>
	<Input id="hostname" bind:value={config.hostname} pattern="^[a-zA-Z][a-zA-Z\d-]{(1, 32)}[a-zA-Z\d]$" placeholder="changeme" minlength="3" maxlength="32" />
</FormGroup>
<FormGroup>
	<Input id="enableWifi" bind:checked={config.enableWifi} type="checkbox" label="Enable WiFi" />
	<Input id="enableSoftAp" bind:checked={config.enableSoftAp} type="checkbox" label="Create AP if no WiFi is available" />
	<Input id="enableBle" bind:checked={config.enableBle} type="checkbox" label="Enable Bluetooth (BLE)" />
	<Input id="enableDac" bind:checked={config.enableDac} type="checkbox" label="Enable DAC Analog Output" />
</FormGroup>
<FormGroup>
	<Input id="autoAirPump" bind:checked={config.autoAirPump} type="checkbox" label="Enable automatic tube repressurizing" />
	<Label for="pressureThresh">Activate on pressure change in hPa</Label>
	<Input id="pressureThresh" bind:value={config.pressureThresh} placeholder="10" min="1" max="65535" type="number" />
	<Input id="airPumpOnBoot" bind:checked={config.airPumpOnBoot} type="checkbox" label="Enable Air Pump on Power-On" />
</FormGroup>
<FormGroup>
	<Input id="otaWebEnabled" bind:checked={config.otaWebEnabled} type="checkbox" label="Enable automatic updates from web (requires internet connectiviy)" />
	<Label for="otaPassword">OTA (Over The Air) firmware update password</Label>
	<Input id="otaPassword" bind:value={config.otaPassword} placeholder="OTA Password" maxlength="32" />
	<Label for="otaWebUrl">OTA Web URL (http://server.tdl/directory)</Label>
	<Input id="otaWebUrl" bind:value={config.otaWebUrl} placeholder="http://server.tdl/directory" minlength="10" maxlength="256" />
</FormGroup>
<FormGroup>
	<Input id="enableMqtt" bind:checked={config.enableMqtt} type="checkbox" label="Publish to MQTT Broker" />
	<Label for="mqttHost">MQTT Host or IP</Label>
	<Input id="mqttHost" bind:value={config.mqttHost} placeholder="mqtt.net.local" maxlength="32" />
	<Label for="mqttPort">MQTT Port (default 1883)</Label>
	<Input id="mqttPort" bind:value={config.mqttPort} placeholder="1883" min="1" max="65535" type="number" />
	<Label for="mqttTopic">MQTT Topic</Label>
	<Input id="mqttTopic" bind:value={config.mqttTopic} placeholder="some/sensor" maxlength="32" />
	<Label for="mqttUser">MQTT Username</Label>
	<Input id="mqttUser" bind:value={config.mqttUser} placeholder="Username" maxlength="32" />
	<Label for="mqttPass">MQTT Password</Label>
	<Input id="mqttPass" bind:value={config.mqttPass} placeholder="Password" maxlength="32" />
</FormGroup>
<Button on:click={doSaveSettings} block style="height: 5rem;"><Fa icon={faFloppyDisk} />&nbsp;Save Settings</Button>
