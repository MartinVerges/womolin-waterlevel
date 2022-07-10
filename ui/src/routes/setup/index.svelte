<script>
  import { browser } from '$app/env';
  import { onMount } from 'svelte';
  import { Button } from 'sveltestrap';
  import { toast } from '@zerodevx/svelte-toast';

  import Fa from 'svelte-fa/src/fa.svelte';
  import { faTriangle } from '@fortawesome/pro-solid-svg-icons/faTriangle';
  import { faRectangle } from '@fortawesome/pro-solid-svg-icons/faRectangle';

  let numSensors = 0;
    onMount(async () => {
		const response = await fetch(`/api/level/num`, {
      headers: { "Content-type": "application/json" }
    }).catch(error => console.log(error))
    if(response.ok) {
      let data = await response.json();
      numSensors = data.num || 0;
    } else {
      toast.push(`Error ${response.status} ${response.statusText}<br>Unable to receive current settings.`, variables.toast.error)
    }
	});

  let selectedSensor = 0;
  let selectedShape = 0;
  let setupSession = undefined;
  if (browser) {
    setupSession = JSON.parse(window.sessionStorage.getItem('setupSession')) ?? { sensor: 0, type: undefined }
  }
  function loadNextStep() {
    setupSession.sensor = selectedSensor;
    setupSession.shape = selectedShape;
    if (browser) window.sessionStorage.setItem('setupSession', JSON.stringify(setupSession));
    if (selectedShape == 1) window.location.href = '/setup/uniform/';
    else if (selectedShape == 2) window.location.href = '/setup/unevenly/';
  }
</script>

<svelte:head>
  <title>Sensor Calibration</title>
</svelte:head>

{#if setupSession}
<div class="container">
  <h4>Calibration - Step 1</h4>
  <p>
      We have to calibrate the sensor in order to use it propperly.
      This usually only needs to be done once, insofar as you are not changing the tank.
  </p>
  <p>
    Which sensor do you want to calibrate?
  </p>
  <p>
    <select bind:value={selectedSensor} id="Sensor" class="form-select">
        <option value="{undefined}" selected="selected">please select a Sensor</option>
    {#each {length: numSensors} as _, i}
        <option value="{(i+1)}">{(i+1)}. Sensor</option>
    {/each}
    </select>
  </p>
  {#if selectedSensor > 0}
  <p>
    Is it a tank that has been formed uniformly so that the water level rises linearly,
    or does the tank have significant bulges, size changes, or other volume changes?
  </p>
  <div class="row">
    <div class="col text-center" on:click={() => selectedShape = 1}>
      <input type="radio" name="shape" value="{1}" bind:group={selectedShape} /><br>
      <Fa icon={faRectangle} size="10x" />
      <p>uniformly shaped</p>
    </div>
    <div class="col text-center" on:click={() => selectedShape = 2}>
      <input type="radio" name="shape" value="{2}" bind:group={selectedShape} /><br>
      <Fa icon={faTriangle} size="10x" />
      <p>unevenly shaped</p>
    </div>
  </div>
  {/if}
  {#if selectedSensor > 0 && selectedShape > 0}
  <p>
    <Button on:click={loadNextStep} block>Start calibration</Button>
  </p>
  {/if}
</div>
{:else}
<h4>Loading, please wait...</h4>
{/if}
