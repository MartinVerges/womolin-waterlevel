<script>
	import '../app.scss';
	import Nav from '../lib/Nav.svelte';
    import Footer from "../lib/Footer.svelte";
	import { SvelteToast } from '@zerodevx/svelte-toast'
</script>

<Nav />
<div class="container mt-5"></div>
<main class="flex-shrink-0 container-md">
	<slot />
</main>
<Footer />
<SvelteToast />
