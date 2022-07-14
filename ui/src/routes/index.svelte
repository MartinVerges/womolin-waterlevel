
<script>
    import { variables } from '$lib/utils/variables';
	import { fade } from 'svelte/transition';
    import { Progress, Button } from 'sveltestrap';
    import { toast } from '@zerodevx/svelte-toast';
    import { onMount } from 'svelte';

    import Fa from 'svelte-fa/src/fa.svelte';
    import { faSync } from '@fortawesome/pro-solid-svg-icons/faSync';

    // ******* SHOW LEVEL ******** //
    let level = {};
    async function getCurrentLevel() {
        const response = await fetch(`/api/level/current/all`, {
            headers: { "Content-type": "application/json" }
        }).catch(error => console.log(error));
        if(response.ok) level = await response.json();
        else {
            toast.push(`Error ${response.status} ${response.statusText}<br>Unable to request current level.`, variables.toast.error)
        }
    }

    let refreshInterval
    onMount(async () => { 
        getCurrentLevel()
        clearInterval(refreshInterval)
        refreshInterval = setInterval(getCurrentLevel, 5000)
    });

    // ******* REPRESSUREIZE ******** //
    let isVisible = false;
    async function repressurizeTube(sensor) {
        isVisible = true;
        const response = await fetch(`/api/restore/pressure?sensor=${sensor}`, {
			method: 'POST',
            headers: { "Content-type": "application/json" }
		}).catch(error => console.log(error));
		if(!response.ok) {
            toast.push(`Error ${response.status} ${response.statusText}<br>Unable to repressurize tube of Sensor ${sensor}.`, variables.toast.error)
        }
        isVisible = false;
    }
</script>

<svelte:head>
  <title>Sensor Status</title>
</svelte:head>

<div class="container">
    <div class="row">
    {#if level == undefined}
        <div class="col">Requesting current level, please wait...</div>
    {:else}
        {#each {length: level.length} as _, i}
        <div class="col-sm-12">Current level of tank sensor {(i+1)}</div>
        <div class="col-sm-9">
            <Progress animated value={level[i]} style="height: 5rem;">{level[i]}%</Progress>
        </div>
        <div class="col-sm-3">
            <Button on:click={()=>repressurizeTube(i+1)} block style="height: 5rem;">
                {#if isVisible}<div out:fade={{ delay: 5000 }}><Fa icon={faSync} size="2x" spin />&nbsp;</div>{/if}
                Repressurize
            </Button>
        </div>
        {/each}
    {/if}
    </div>
</div>
