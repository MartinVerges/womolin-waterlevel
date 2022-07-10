
<script>
    import { variables } from '$lib/utils/variables';
  	import { onMount } from 'svelte';
    import { Button, Form, FormGroup, Input, Label } from 'sveltestrap';
    import Fa from 'svelte-fa/src/fa.svelte';
    import { faFloppyDisk } from '@fortawesome/pro-solid-svg-icons/faFloppyDisk';
    import { toast } from '@zerodevx/svelte-toast';

    let config = {};

    onMount(async () => {
		const response = await fetch(`/api/config`, {
            headers: { "Content-type": "application/json" }
        })
        .catch(error => { throw new Error(`${error})`); });
        if(response.ok) config = await response.json();
        else {
            toast.push(`Error ${response.status} ${response.statusText}<br>Unable to receive current settings.`, variables.toast.error)
        }
	});

    async function doSaveSettings () {
		fetch(`/api/config`, {
			method: 'POST',
			body: JSON.stringify(config),
            headers: { "Content-type": "application/json" }
		}).then(response => {
            if (response.ok) {
                toast.push(`Settings successfully saved`, variables.toast.success)
            } else {
                toast.push(`Error ${response.status} ${response.statusText}<br>Unable to store new AP configuration.`, variables.toast.error)
            }
        }).catch(error => { throw new Error(`${error})`); })		
	}
</script>

<svelte:head>
  <title>Sensor Settings</title>
</svelte:head>

<div class="container">
    <h4>Configuration</h4>
    <Form>
        <FormGroup>
            <Label for="hostname">Hostname</Label>
            <Input id="hostname" bind:value={config.hostname} pattern="^[a-zA-Z][a-zA-Z\d-]{1,32}[a-zA-Z\d]$" placeholder="changeme"  minlength="3" maxlength="32"/>
        </FormGroup>
        <FormGroup>
            <Input id="enablewifi" bind:checked={config.enablewifi} type="checkbox" label="Enable WiFi" />
            <Input id="enablesoftap" bind:checked={config.enablesoftap} type="checkbox" label="Create AP if no WiFi is available" />
            <Input id="enableble" bind:checked={config.enableble} type="checkbox" label="Enable Bluetooth (BLE)" />
            <Input id="enabledac" bind:checked={config.enabledac} type="checkbox" label="Enable DAC Analog Output" />
        </FormGroup>
        <FormGroup>
            <Input id="enablemqtt" bind:checked={config.enablemqtt} type="checkbox" label="Publish to MQTT Broker" />
            <Label for="mqtthost">MQTT Host or IP</Label>
            <Input id="mqtthost" bind:value={config.mqtthost} placeholder="mqtt.net.local" maxlength="32"/>
            <Label for="mqttport">MQTT Port (default 1883)</Label>
            <Input id="mqttport" bind:value={config.mqttport} placeholder="1883" min="1" max="65535" type="number"/>
            <Label for="mqtttopic">MQTT Topic</Label>
            <Input id="mqtttopic" bind:value={config.mqtttopic} placeholder="some/sensor" maxlength="32"/>
            <Label for="mqttuser">MQTT Username</Label>
            <Input id="mqttuser" bind:value={config.mqttuser} placeholder="Username" maxlength="32"/>
            <Label for="mqttpass">MQTT Password</Label>
            <Input id="mqttpass" bind:value={config.mqttpass} placeholder="Password" maxlength="32"/>
        </FormGroup>
    </Form>
    <Button on:click={doSaveSettings} block style="height: 5rem;"><Fa icon={faFloppyDisk} />&nbsp;Save Settings</Button>
</div>
