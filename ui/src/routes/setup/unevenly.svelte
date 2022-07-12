
<script>
  import { variables } from '$lib/utils/variables';
  import { browser } from '$app/env';
  import { onMount } from 'svelte';
  import { 
    Button,
    Input,
    Label
  } from 'sveltestrap';
  import { toast } from '@zerodevx/svelte-toast';

  let setupSession = undefined;
  if (browser) {
    setupSession = JSON.parse(window.sessionStorage.getItem('setupSession')) ?? { sensor: 0 }
  }

  // ******* SHOW RAW SENSOR VALUE ******** //
  let sensorValue = {};
  async function getCurrentRawValue (id) { 
		const response = await fetch(`/api/rawvalue?sensor=${setupSession.sensor}`, {
      headers: { "Content-type": "application/json" }
    }).catch(error => console.log(error))
    if(response.ok) {
      let data = await response.json();
      sensorValue = data.raw || undefined;
    } else {
      toast.push(`Error ${response.status} ${response.statusText}<br>Unable to read raw sensor value.`, variables.toast.error)
    }
  }

  let refreshInterval
  onMount(async () => { 
    getCurrentRawValue()
    clearInterval(refreshInterval)
    refreshInterval = setInterval(getCurrentRawValue, 5000)
  });

  function endCalibration() {
    if (browser) window.sessionStorage.removeItem('setupSession');

    fetch(`/api/setup/end?sensor=${setupSession.sensor}`, {
      method: 'POST', body: {}, headers: { "Content-type": "application/json" }
    }).then(response => {
      if (response.ok) {
        toast.push(`Calibration successfully saved`, {
          theme: variables.toast.success.theme,
          onpop: (id) => { window.location.href = '/'; }
        })
      } else {
        toast.push(`Error ${response.status} ${response.statusText}<br>Unable to store new calibration.`, variables.toast.error)
      }
    }).catch(error => console.log(error))
  }

  function abortCalibration() {
    if (browser) window.sessionStorage.removeItem('setupSession');

    fetch(`/api/setup/abort?sensor=${setupSession.sensor}`, {
      method: 'POST', body: {}, headers: { "Content-type": "application/json" }
    }).then(response => {
      if (response.ok) {
        toast.push(`Calibration successfully aborted`, {
          theme: variables.toast.success.theme,
          onpop: (id) => { window.location.href = '/'; }
        })
      } else {
        toast.push(`Error ${response.status} ${response.statusText}<br>Unable to abort the calibration.`, variables.toast.error)
      }
    }).catch(error => console.log(error))
  }

  let begin = false;
  function beginCalibration() {
    fetch(`/api/setup/start?sensor=${setupSession.sensor}`, {
      method: 'POST', body: {}, headers: { "Content-type": "application/json" }
    }).then(response => {
      if (response.ok) {
        toast.push('Prepare to open up your inflow', {
          duration: 5000,
          initial: 0,
          next: 1,
          dismissable: false,
          theme: variables.toast.success.theme,
          intro: { y: 192 },
          onpop: () => begin = true,
        })
      } else {
        toast.push(`Error ${response.status} ${response.statusText}<br>Unable to start new calibration.`, variables.toast.error)
      }
    }).catch(error => console.log(error))
  }
</script>

<svelte:head>
  <title>Sensor Calibration</title>
</svelte:head>

{#if setupSession && setupSession.sensor > 0}
<h4>Calibration - Step 2 {#if setupSession.sensor > 0} of the sensor {setupSession.sensor}{/if}</h4>
{#if begin == false}
<p>
  You have chosen to calibrate the sensor in a unevenly shaped tank.
  This tank shape requires a more accurate measurement of the volume.
  Here, we record the level over the measurement period and translate the result into a uniform percentage level.
  This way you always know how much liquid is left in the tank.
</p>
<p>
  To perform the calibration, please make sure that your vehicle is level.
  In particular, you will need a water source that provides a steady flow.
  Typical potable water connections will suffice.
  Make sure that you can start refueling your vehicle as soon as indicated.
</p>
<p>
  <Button on:click={beginCalibration} block>Click here to begin the calibration</Button>
</p>
{:else}
<p>The measurement is now performed. <strong>Open the inflow now!</strong></p>
<p>
  Ensure a steady inflow and wait until the tank has been completely filled. 
  Once the tank has been completely filled. 
  Turn off the water tap and then finish the measurement process.
</p>
<p>
  <Label for="rawvalue">Current RAW sensor value</Label>
  <Input id="rawvalue" bind:value={sensorValue} type="number" disabled/>
</p>
<div class="row">
  <div class="col-sm-6">
    <Button on:click={endCalibration} class="btn-success" block>End measuring process</Button>
  </div>
  <div class="col-sm-6">
    <Button on:click={abortCalibration} class="btn-danger" block>Abort measuring process</Button>
  </div>
</div>
{/if}
{:else if setupSession}
<h4>Unknown state, please restart calibration.</h4>
{:else}
<h4>Loading, please wait...</h4>
{/if}
