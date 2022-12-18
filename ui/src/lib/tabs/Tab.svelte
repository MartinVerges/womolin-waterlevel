<script>
	import { getContext } from 'svelte';
	import { tabContextId } from './Tabs.svelte';

	export let id;

	const { selectedTab, selectTab } = getContext(tabContextId);
</script>

<li class="nav-item">
	<button class="nav-link" class:active={$selectedTab === id} on:click={() => selectTab(id)}>
		<slot />
	</button>
</li>
