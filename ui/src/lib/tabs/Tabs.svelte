<script context="module">
	export const tabContextId = new Set();
</script>

<script>
	import { setContext } from 'svelte';
	import { writable } from 'svelte/store';
	
	export let selectedId;
	
	const selectedTab = writable(selectedId);
	
	$: selectedTab.set(selectedId);

	setContext(tabContextId, {
		selectTab: tabId => selectedId = tabId,
		selectedTab,
	});
</script>

<div class="tabs">
	<slot></slot>
</div>