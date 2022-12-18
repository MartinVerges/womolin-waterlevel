<script>
	import { onMount } from 'svelte';

	let log = '';
	onMount(async () => {
		let current_loc = window.location;
		let use_proto = current_loc.protocol === 'https:' ? 'wss' : 'ws';
		let ws_uri = use_proto + '://' + current_loc.host + '/api/webserial';
		const socket = new WebSocket(ws_uri);

		socket.onclose = () => {
			console.log('WebSocket to the sensor closed');
			log += '\nConnection closed!\n';
		};

		socket.onopen = () => {
			console.log('WebSocket to the sensor opened');
			log = 'Connection ready...\n';
		};

		socket.onmessage = (event) => {
			log += event.data;
		};

		socket.onerror = (event) => {
			log += '\nERROR: ' + event;
		};
	});
</script>

<svelte:head>
	<title>WebSerial Console of the Sensor</title>
</svelte:head>

<h4>Serial Console of the Sensor</h4>

<pre>
{#if log == ''}
		loading ...
	{:else}
		{log}
	{/if}
</pre>
