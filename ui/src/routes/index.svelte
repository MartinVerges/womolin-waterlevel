
<script>
    import { variables } from '$lib/utils/variables';
	import { fade } from 'svelte/transition';
    import { Progress, Button, Spinner } from 'sveltestrap';

    // ******* SHOW LEVEL ******** //
    let level = undefined;
    async function getCurrentLevel() {
        const res = await fetch(`${variables.url}/api/level/current`);
        const value = await res.text();

        if (res.ok) level = parseInt(value);
		else throw new Error(value);
    }
    $: getCurrentLevel()

    let refreshInterval
    $: {
        clearInterval(refreshInterval)
        refreshInterval = setInterval(getCurrentLevel, 2500)
    }

    // ******* REPRESSUREIZE ******** //
    let isVisible = false;
    async function repressurizeTube() {
        isVisible = true;
        fetch(`${url}/api/restore/pressure`)
        .then(response => {
            isVisible = false;
        })
        .catch(error => console.log(error));
    }
</script>

<div class="container">
    <div class="row">
        {#if level == undefined}
        <div class="col">Requesting current tank level, please wait...</div>
        {:else}
        <div class="col-sm-12">Current tank level:</div>
        <div class="col-sm-9">
            <Progress animated value={level} style="height: 5rem;">{level}%</Progress>
        </div>
        <div class="col-sm-3">
            <Button on:click={repressurizeTube} block style="height: 5rem;">
                {#if isVisible}<div in:fade={{ delay: 5000 }}><Spinner size="sm" type="grow" />&nbsp;</div>{/if}
                Repressurize
            </Button>
        </div>
        {/if}
    </div>
</div>
