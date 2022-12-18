<script>
	import Header from './Header.svelte';
	import Footer from './Footer.svelte';
	import { SvelteToast } from '@zerodevx/svelte-toast';
</script>

<Header />
<div class="container mt-5" />
<main class="flex-shrink-0 container-md">
	<slot />
</main>
<Footer />
<SvelteToast />
