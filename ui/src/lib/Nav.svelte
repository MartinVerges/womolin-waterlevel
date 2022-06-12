
<script>
    import Fa from 'svelte-fa/src/fa.svelte';
    import { 
        faRv,
        faHouseChimney,
        faWifi,
        faGear
    } from '@fortawesome/pro-solid-svg-icons';
    import { fly } from 'svelte/transition';
    import { page } from '$app/stores';
    let path;
    $: path = $page.url.pathname;

    // Do a mouseover animation on the icon, just for fun!
    let visibleIcon = true;
    const showIcon = () => visibleIcon = true
    const hideIcon = () => visibleIcon = false
</script>

<header>
    <div class="px-3 py-2 bg-dark text-white">
      <div class="container">
        <div class="d-flex flex-wrap align-items-center justify-content-center justify-content-lg-start">
          {#if visibleIcon}
          <a href="/" class="d-flex align-items-center my-2 my-lg-0 me-lg-auto text-white text-decoration-none" 
           out:fly="{{ x: 500, duration: 6000 }}"
           in:fly="{{ x: -100, duration: 2000 }}"
           on:outroend="{showIcon}"
           on:mouseenter="{hideIcon}"
           ><Fa id="mainicon" icon={faRv} size="3x" /></a>
          {/if}

          <ul class="nav col-12 col-lg-auto my-2 justify-content-center my-md-0 text-small">
            <li>
              <a href="/" class="nav-link {path === '/' ? 'text-secondary' : 'text-white'}">
                <Fa icon={faHouseChimney} size="lg" class="bi d-block mx-auto mb-1" />Home
              </a>
            </li>
            <li>
              <a href="/wifi" class="nav-link {path === '/wifi' ? 'text-secondary' : 'text-white'}">
                <Fa icon={faWifi} size="lg" class="bi d-block mx-auto mb-1" />Wifi
              </a>
            </li>
            <li>
              <a href="/settings" class="nav-link {path === '/settings' ? 'text-secondary' : 'text-white'}">
                <Fa icon={faGear} size="lg" class="bi d-block mx-auto mb-1" />Settings
              </a>
            </li>
          </ul>
        </div>
      </div>
    </div>
  </header>
