<script>
	import { variables } from '$lib/utils/variables';
	import { browser } from '$app/environment';
	import { Button, Input, Label } from 'sveltestrap';
	import { toast } from '@zerodevx/svelte-toast';

	let config = {
		lower: undefined,
		upper: undefined,
		volume: undefined,
		unit: undefined
	};

	let setupSession = undefined;
	if (browser) {
		setupSession = JSON.parse(window.sessionStorage.getItem('setupSession')) ?? { sensor: 0 };
	}

	function exitCalibration() {
		if (browser) window.sessionStorage.removeItem('setupSession');

		fetch(`/api/setup/values?sensor=${setupSession.sensor}`, {
			method: 'POST',
			body: JSON.stringify(config),
			headers: { 'Content-type': 'application/json' }
		})
			.then((response) => {
				if (response.ok) {
					toast.push(`Calibration successfully saved`, {
						theme: variables.toast.success.theme,
						onpop: () => {
							window.location.href = '/';
						}
					});
				} else {
					toast.push(`Error ${response.status} ${response.statusText}<br>Unable to store new calibration.`, variables.toast.error);
				}
			})
			.catch((error) => console.log(error));
	}

	async function getCurrentRawValue(id) {
		const response = await fetch(`/api/rawvalue?sensor=${setupSession.sensor}`, {
			headers: { 'Content-type': 'application/json' }
		}).catch((error) => console.log(error));
		if (response.ok) {
			let data = await response.json();
			if (id == 'upper') config.upper = data.raw || undefined;
			else if (id == 'lower') config.lower = data.raw || undefined;
		} else {
			toast.push(`Error ${response.status} ${response.statusText}<br>Unable to read raw sensor value.`, variables.toast.error);
		}
	}
</script>

<svelte:head>
	<title>Sensor Calibration</title>
</svelte:head>

{#if setupSession && setupSession.sensor > 0}
	<h4>
		Calibration - Step 2 {#if setupSession.sensor > 0} of the sensor {setupSession.sensor}{/if}
	</h4>
	<p>You have chosen to calibrate the sensor in a uniformly shaped standard tank.</p>
	<p>The calibration is performed by a measurement when the tank is empty and another measurement after the tank is completely filled.</p>

	<div class="row">
		<div class="col-sm-6">
			<Label for="lower">How much liquid in liters can you fit in your tank.</Label>
			<Input id="lower" bind:value={config.volume} type="number" />
		</div>
		<div class="col-sm-6">
			<Label for="unit">In what unit is your value</Label>
			<select bind:value={config.unit} id="unit" class="form-select">
				<option value="liters" selected="selected">Liters (l)</option>
				<option value="milliliters">Milliliters (ml)</option>
				<option value="usgallons">US Liquid Gallons (US gal lqd)</option>
			</select>
		</div>
	</div>
	<br />
	<div class="row">
		<div class="col-sm-9">
			<Label for="lower">Please make sure that the tank has been completely emptied and then press the button to determine the sensor value.</Label>
			<Input id="lower" bind:value={config.lower} type="number" />
		</div>
		<div class="col-sm-3">
			<Button block class="h-100" on:click={() => getCurrentRawValue('lower')}>Read current sensor value</Button>
		</div>
	</div>
	<br />
	<div class="row">
		<div class="col-sm-9">
			<Label for="upper">Fill the tank to the maximum, wait a short moment for the water to settle, and then press the button.</Label>
			<Input id="upper" bind:value={config.upper} type="number" />
		</div>
		<div class="col-sm-3">
			<Button block class="h-100" on:click={() => getCurrentRawValue('upper')}>Read current sensor value</Button>
		</div>
	</div>
	<br />
	{#if config.lower && config.upper}
		<p>
			<Button on:click={exitCalibration} block>Save and Exit calibration</Button>
		</p>
	{/if}
{:else if setupSession}
	<h4>Unknown state, please restart calibration.</h4>
{:else}
	<h4>Loading, please wait...</h4>
{/if}
