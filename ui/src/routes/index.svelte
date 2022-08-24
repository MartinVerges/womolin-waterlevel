
<script>
  import { variables } from '$lib/utils/variables';
  import { fade } from 'svelte/transition';
  import { Progress, Button } from 'sveltestrap';
  import { toast } from '@zerodevx/svelte-toast';
  import { onMount } from 'svelte';

  import Fa from 'svelte-fa/src/fa.svelte';
  import { faSync } from '@fortawesome/pro-solid-svg-icons/faSync';

  // ******* SHOW LEVEL ******** //
  let level = undefined;
  //level = [ 77, 22 ]
  onMount(async () => {
    // initial level
    const response = await fetch(`/api/level/current/all`).catch(error => console.log(error));
    if(response.ok) level = await response.json();
    else {
      toast.push(`Error ${response.status} ${response.statusText}<br>Unable to request current level.`, variables.toast.error)
    }
  });

  onMount(async () => {
    // dynamic refreshing level
    if (!!window.EventSource) {
      var source = new EventSource('/api/events');

      source.addEventListener('error', function(e) {
        if (e.target.readyState != EventSource.OPEN) {
          console.log("Events Disconnected");
        }
      }, false);

      source.addEventListener('message', function(e) {
        console.log("message", e.data);
      }, false);

      source.addEventListener('status', function(e) {
        try {
          let data = JSON.parse(e.data);          
          level = [];
          data.forEach((element, i) => {
            if ('level' in element) level[i] = element.level
          }); 
        } catch (error) {
          console.log(error);
          console.log("Error parsing status", e.data);          
        }
      }, false);
    }
  })

    // ******* REPRESSUREIZE ******** //
    let isVisible = [];
    async function repressurizeTube(sensor) {
      isVisible[sensor] = true;
      const response = await fetch(`/api/restore/pressure?sensor=${sensor+1}`, {
			  method: 'POST',
        headers: { "Content-type": "application/json" }
		  }).catch(error => console.log(error));
		  if(!response.ok) {
        toast.push(`Error ${response.status} ${response.statusText}<br>Unable to repressurize tube of Sensor ${sensor+1}.`, variables.toast.error)
      }
      isVisible[sensor] = false;
    }[sensor]
</script>

<svelte:head>
  <title>Sensor Status</title>
</svelte:head>

<div class="row">
{#if level == undefined}
  <div class="col">Requesting current level, please wait...</div>
{:else if level.length == 0}
<div class="col">Please calibrate your Sensor...</div>
{:else}
  {#each {length: level.length} as _, i}
  <div class="col-sm-12">Current level of tank sensor {(i+1)}</div>
  <div class="col-sm-9">
    <Progress animated value={level[i]} style="height: 5rem;">{level[i]}%</Progress>
  </div>
  <div class="col-sm-3">
    <Button on:click={()=>repressurizeTube(i)} block style="height: 5rem;">
      {#if isVisible[i]}<div out:fade={{ delay: 5000 }}><Fa icon={faSync} size="2x" spin />&nbsp;</div>{/if}
      Repressurize
    </Button>
  </div>
  {/each}
{/if}
</div>
