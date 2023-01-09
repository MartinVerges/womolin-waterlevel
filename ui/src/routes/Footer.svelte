<script>
	import Fa from 'svelte-fa/src/fa.svelte';
	import { faRectangleTerminal } from '@fortawesome/pro-solid-svg-icons/faRectangleTerminal';
	import { faMemoCircleInfo } from '@fortawesome/pro-solid-svg-icons/faMemoCircleInfo';
	import { faFileImport } from '@fortawesome/pro-solid-svg-icons/faFileImport';
	import { onMount } from 'svelte';

	// ******* SHOW FIRMWARE INFO ******** //
	let firmware = undefined;
	onMount(async () => {
		// initial level
		const response = await fetch(`/api/firmware/info`, {
			headers: { 'Content-type': 'application/json' }
		}).catch((error) => console.log(error));
		if (response.ok) {
			firmware = await response.json();
		}
	});

	import { page } from '$app/stores';
	let path;
	$: path = $page.url.pathname;
</script>

<div class="container">
	<footer class="d-flex flex-wrap justify-content-between align-items-center py-3 my-4 border-top">
		<div class="col-md-4 d-flex align-items-center">
			<span class="text-muted">© 2022 by Martin Verges</span>
		</div>

		{#if firmware != undefined}
			<div class="nav col-md-4 d-flex align-items-center text-muted">
				<span><small>{firmware.firmware_version} ({firmware.firmware_date})</small></span>
			</div>
		{/if}

		<ul class="nav col-md-4 justify-content-end list-unstyled d-flex">
			<li class="ms-3"><a class="text-muted" href="/console/"><Fa icon={faRectangleTerminal} class="mx-1" />Console</a></li>
			<li class="ms-3"><a class="text-muted" href="/info/"><Fa icon={faMemoCircleInfo} class="mx-1" />Info</a></li>
			<li class="ms-3"><a class="text-muted" href="/update/"><Fa icon={faFileImport} class="mx-1" />Update</a></li>
		</ul>
	</footer>
</div>
