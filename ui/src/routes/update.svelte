
<script>
  import { variables } from '$lib/utils/variables';
  import { Button, Progress, FormGroup, FormText, Input, Label } from 'sveltestrap';
  import Fa from 'svelte-fa/src/fa.svelte';
  import { faUpload } from '@fortawesome/pro-solid-svg-icons/faUpload';
  import { toast } from '@zerodevx/svelte-toast';

  let uploadPercentCompleted = 0
  let otaPassword = "";

  function onSubmit(event) {
    let data = new FormData();
    data.append('file', document.querySelector('#firmware').files[0]);

    let request = new XMLHttpRequest();
    request.open('POST', '/api/update/upload');
    if (otaPassword.length>0) request.setRequestHeader("Authorization", "Basic " + window.btoa("ota:"+otaPassword));

    request.upload.onprogress = event => {
      uploadPercentCompleted = Math.round((event.loaded / event.total) * 100);
    }

    request.onerror = event => {
      uploadPercentCompleted = 0;
      toast.push(`Error ${request.status} ${request.statusText}<br>Unable to upload the firmware`, variables.toast.error)
      request.abort()
    }

    request.onload = event => {
     if (request.status == 200) {
        toast.push(`New firmware uploaded, please wait!`, {
          theme: variables.toast.success.theme,
          duration: 15000,
          onpop: (id) => { window.location.href = '/'; }
        })
      } else if (request.status == 401) {
        uploadPercentCompleted = 0;
        toast.push(`Error ${request.status} ${request.statusText}<br>Wrong OTA password provided`, variables.toast.error)
      } else {
        let jsonResponse = JSON.parse(request.response);
        toast.push(`Error ${request.status} ${jsonResponse.error}<br>Unable to upload firmware file`, variables.toast.error)
      }
    }

    // send POST request to server
    request.send(data);
  }
</script>

<svelte:head>
  <title>Upload a new Firmware</title>
</svelte:head>

<h4>OTA (Over the Air) Firmware Update</h4>
{#if uploadPercentCompleted > 0}
<p>
  Please wait.
  The firmware update will now be loaded onto the sensor. 
</p>
<Progress animated value={uploadPercentCompleted} style="height: 5rem;">{Math.round(uploadPercentCompleted)}%</Progress>  
{:else}
<p>
  You can conveniently install a new firmware version of the sensor here.
  Please make sure that your file is suitable for this type and has been saved correctly on your computer.
</p>
<p>
  If the upload fails, the previous firmware will still start.
  If again no start is possible, you have to install a new firmware via USB cable.
</p>
<form on:submit|preventDefault={onSubmit} method="POST" enctype="multipart/form-data">
  <FormGroup>
    <Label for="firmware">The Firmware file</Label>
    <Input type="file" name="update_package" id="firmware" accept=".bin"/>
    <FormText color="muted">
      Please provide the correct firmware file to update your sensor using OTA mechanism.
    </FormText>
  </FormGroup>
  <FormGroup>
    <Label for="otaPassword">The required OTA password</Label>
    <Input id="otaPassword" bind:value={otaPassword} type="password" label="OTA password" placeholder="OTA password"/>
  </FormGroup>
  <Button block style="height: 5rem;"><Fa icon={faUpload} />&nbsp;Upload the file to the Sensor</Button>
</form>
{/if}