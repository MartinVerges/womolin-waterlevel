
<script>
	import { getContext } from 'svelte';
	import { tabContextId } from './Tabs.svelte';

	export let id;
	
	const { selectedTab } = getContext(tabContextId);
</script>

{#if $selectedTab === id}
<div class="tab-content mt-4">
	<slot></slot>
</div>
{/if}
