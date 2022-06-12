
<script>
    import { variables } from '$lib/utils/variables';
  	import { onMount } from 'svelte';
    import { 
        Button,
        InputGroup,
        InputGroupText,
        Input,
        Modal,
        ModalBody,
        ModalFooter,
        ModalHeader
    } from 'sveltestrap';
    import Fa from 'svelte-fa/src/fa.svelte';
    import {
        faWifi,
        faFloppyDisk,
        faSignalBars,
        faSignalBarsGood,
        faSignalBarsFair,
        faSignalBarsWeak,
        faLockKeyhole,
        faLockKeyholeOpen
    } from '@fortawesome/pro-solid-svg-icons';
    import { toast } from '@zerodevx/svelte-toast'
    import Tabs from '$lib/tabs/Tabs.svelte';
	import TabPanel from '$lib/tabs/TabPanel.svelte';
	import TabList from '$lib/tabs/TabList.svelte';
	import Tab from '$lib/tabs/Tab.svelte';
    let selectedId = "wifiConfigList";

    let wifiConfigList = []; // [{"id":1,"apName":"xxx","apPass":true},...]
    onMount(async () => {
		const res = await fetch(`${variables.url}/api/wifi/configlist`)
        .catch(error => {
            throw new Error(`${error})`);
        });
		if(res.ok) wifiConfigList = await res.json();
        else {
            toast.push(`Error ${response.status} ${response.statusText}<br>Unable to request the list of known Wifi SSIDs.`, {
                theme: { '--toastBackground': '#F56565', '--toastBarBackground': '#C53030' }
            })
        }
	});

    let wifiScanList = []; // [{"ssid":"xxx","encryptionType":3,"rssi":-58,"channel":7},...]
    onMount(async () => {
		const res = await fetch(`${variables.url}/api/wifi/scan`)
        .catch(error => {
            throw new Error(`${error})`);
        });
        if(res.ok) wifiScanList = await res.json();
        else {
            toast.push(`Error ${response.status} ${response.statusText}<br>Unable to scan for Wifi Networks.`, {
                theme: { '--toastBackground': '#F56565', '--toastBarBackground': '#C53030' }
            })
        }
	});

    let addApConfig = { apName: "", apPass: "" };
    async function doAddAp () {
		fetch(`${variables.url}/api/wifi/add`, {
			method: 'POST',
			body: JSON.stringify(addApConfig),
            headers: { "Content-type": "application/json" }
		}).then(response => {
            if (response.ok) {
                // clear fields after successfull POST
                addApConfig.apName = "";
                addApConfig.apPass = "";
                toast.push(`The new AP was saved`, {
                    theme: { '--toastBackground': '#48BB78', '--toastBarBackground': '#2F855A'
                    }
                })
            } else {
                toast.push(`Error ${response.status} ${response.statusText}<br>Unable to store new AP configuration.`, {
                    theme: { '--toastBackground': '#F56565', '--toastBarBackground': '#C53030' }
                })
            }
        }).catch(error => {
            throw new Error(`${error})`);
        })
	}

    function prefillNewAp(ssid) {
        selectedId = "wifiScanList"; // WTF how can that be avoided!
        addApConfig.apName = ssid;
        selectedId = "wifiAddAp";
    }

    let deleteAp = { id: undefined };
    let openModal = false;
    const toggleModal = () => openModal = !openModal;
    async function doDeleteAp () {
        openModal = false;
		fetch(`${variables.url}/api/wifi/id`, {
			method: 'DELETE',
			body: JSON.stringify(deleteAp),
            headers: { "Content-type": "application/json" }
		}).then(response => {
            if (response.ok) {
                openModal = false;
                toast.push(`Successfully removed AP from known List`, {
                    theme: { '--toastBackground': '#48BB78', '--toastBarBackground': '#2F855A'
                    }
                })
            } else {
                toast.push(`Error ${response.status} ${response.statusText}<br>Unable to remove the AP configuration.`, {
                    theme: { '--toastBackground': '#F56565', '--toastBarBackground': '#C53030' }
                })
            }
        }).catch(error => {
            throw new Error(`${error})`);
        })
	}
</script>

<div>
    <Modal isOpen={openModal} {toggleModal}>
      <ModalHeader>Warning</ModalHeader>
      <ModalBody>Are you sure that you want to delete the AP #{deleteAp.id}?</ModalBody>
      <ModalFooter>
        <Button color="secondary" on:click={() => { openModal = false; }}>Cancel</Button>
        <Button color="danger" on:click={doDeleteAp}>Delete WiFi AP</Button>
      </ModalFooter>
    </Modal>
</div>

<Tabs {selectedId}>
	<TabList>
		<Tab id="wifiConfigList" on:click={() => selectedId = "wifiConfigList"}>Known WiFi Networks</Tab>
		<Tab id="wifiScanList">Scan for WiFi Networks</Tab>
		<Tab id="wifiAddAp">Add new SSID</Tab>
	</TabList>

	<TabPanel id="wifiConfigList">
        <h4>List of known Wifi networks</h4>
        {#each wifiConfigList as apEntry}
        <div class="d-flex py-1 align-items-center">
            <div class="p-2"><Fa fw icon={faSignalBars} /></div>
            <div class="p-2 flex-grow-1 flex-lg-grow-0">{apEntry.apName}</div>
            <div class="p-2"><Button class="btn-danger" size="sm" on:click={() => { openModal = true; deleteAp.id = apEntry.id; }}>Delete Wifi</Button></div>
        </div>
        {:else}
        <div class="d-flex"><div class="col-auto">loading ...</div></div>
        {/each}
	</TabPanel>

	<TabPanel id="wifiScanList">
        <h4>Available Wifi networks</h4>
        {#each wifiScanList as apEntry}
        <div class="d-flex py-1 align-items-center">
            <div class="p-2">
                <Fa fw icon={apEntry.rssi > -67 ? faSignalBars
                    : apEntry.rssi > -70 ? faSignalBarsGood
                    : apEntry.rssi > -80 ? faSignalBarsFair
                    : faSignalBarsWeak} />
                <Fa fw icon={apEntry.encryptionType > 0 ? faLockKeyhole : faLockKeyholeOpen} /></div>
            <div class="p-2"><Button size="sm" on:click={() => prefillNewAp(apEntry.ssid)}>Add WiFi AP</Button></div>
            <div class="p-2 flex-grow-1">{apEntry.ssid}</div>
            <div class="p-2">{apEntry.rssi} dbm</div>
        </div>
        {:else}
        <div class="row"><div class="col-auto">loading ...</div></div>
        {/each}
	</TabPanel>

	<TabPanel id="wifiAddAp">
        <h4>Add a new WiFi SSID</h4>
        <InputGroup class="mt-2">
            <InputGroupText><Fa fw icon={faWifi} /></InputGroupText>
            <Input bind:value={addApConfig.apName} placeholder="SSID of the WiFi Network" />
        </InputGroup>
        <InputGroup class="mt-2">
            <InputGroupText><Fa fw icon={faLockKeyhole} /></InputGroupText>
            <Input bind:value={addApConfig.apPass} placeholder="Password to the WiFi" />
        </InputGroup>
        <Button on:click={doAddAp} class="mt-2" block><Fa icon={faFloppyDisk} />&nbsp;Save Settings</Button>
	</TabPanel>
</Tabs>
