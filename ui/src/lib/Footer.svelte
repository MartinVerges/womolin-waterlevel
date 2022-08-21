
<script>
    import Fa from 'svelte-fa/src/fa.svelte';
    import { faRectangleTerminal } from '@fortawesome/pro-solid-svg-icons/faRectangleTerminal'
    import { faMemoCircleInfo } from '@fortawesome/pro-solid-svg-icons/faMemoCircleInfo'
    import { faFileImport } from '@fortawesome/pro-solid-svg-icons/faFileImport';

    import { page } from '$app/stores';
    let path;
    $: path = $page.url.pathname;
  </script>
  
  <div class="container">
    <footer class="d-flex flex-wrap justify-content-between align-items-center py-3 my-4 border-top">
      <div class="col-md-4 d-flex align-items-center">
        <a href="/" class="mb-3 me-2 mb-md-0 text-muted text-decoration-none lh-1">
          <svg class="bi" width="30" height="24"><use xlink:href="#bootstrap"></use></svg>
        </a>
        <span class="text-muted">© 2022 by Martin Verges</span>
      </div>
  
      <ul class="nav col-md-4 justify-content-end list-unstyled d-flex">
        <li class="ms-3"><a class="text-muted" href="/console/"><Fa icon={faRectangleTerminal} class="mx-1" />Console</a></li>
        <li class="ms-3"><a class="text-muted" href="/info/"><Fa icon={faMemoCircleInfo} class="mx-1" />Info</a></li>
        <li class="ms-3"><a class="text-muted" href="/update/"><Fa icon={faFileImport} class="mx-1" />Update</a></li>
      </ul>
    </footer>
  </div>
  