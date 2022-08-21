
<script>
  import { onMount } from 'svelte';

  let data = undefined
  onMount(async () => {
    const response = await fetch(`/api/esp`).catch(error => console.log(error));
    if(response.ok) data = await response.json();
    else {
      toast.push(`Error ${response.status} ${response.statusText}<br>Unable to request sensor information.`, variables.toast.error)
    }
  })
</script>

<svelte:head>
  <title>Internal status of the Sensor</title>
</svelte:head>

<h4>Internal status of the Sensor</h4>

<pre>
{#if data == undefined}
  loading ...
{:else}
  {JSON.stringify(data, null, '  ')}
{/if}
</pre>
