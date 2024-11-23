function toggleLight(isChecked) {
    fetch(`/toggleLight?state=${isChecked}`)
      .then(response => response.text())
      .then(data => {
        console.log(data);
        document.getElementById('lightState').innerText = isChecked ? 'On' : 'Off';
      })
      .catch(error => console.error('Error:', error));
  }

  function selectLightColor(color) {
    fetch(`/setLightColor?color=${encodeURIComponent(color)}`)
      .then(response => response.text())
      .then(data => {
        document.getElementById('lightColorState').innerText = color;
      })
      .catch(error => console.error('Error:', error));
  }

  function toggleDisplayType(isChecked) {
    const stateText = document.getElementById('displayTypeState');
    stateText.innerText = isChecked ? 'Mode: Enhanced' : 'Mode: Standard';
    // Additional logic to switch display types can be added here
  }

  function toggleLightSchedule(isChecked) {
    const container = document.getElementById('lightScheduleContainer');
    if (isChecked) {
      container.style.display = 'block';
    } else {
      container.style.display = 'none';
    }
  }

  function toggleNtpTimeUpdate(isChecked) {
    const container = document.getElementById('ntpTimeUpdateContainer');
    if (isChecked) {
      container.style.display = 'block';
    } else {
      container.style.display = 'none';
    }
  }

  function toggleNtpAutoUpdate(isChecked) {
    const container = document.getElementById('ntpAutoUpdateContainer');
    if (isChecked) {
      container.style.display = 'block';
    } else {
      container.style.display = 'none';
    }
  }

  function toggleHaIntegration(isChecked) {
    const container = document.getElementById('haIntegrationContainer');
    if (isChecked) {
      container.style.display = 'block';
    } else {
      container.style.display = 'none';
    }
  }

  document.addEventListener('DOMContentLoaded', function() {
  // Initialize NTP Time Update
    const ntpToggle = document.getElementById('ntpTimeUpdate');
    if (ntpToggle) {
      toggleNtpTimeUpdate(ntpToggle.checked);
    }

    // Initialize NTP Auto Update
    const ntpAutoUpdateToggle = document.getElementById('ntpAutoUpdate');
    if (ntpAutoUpdateToggle) {
      toggleNtpAutoUpdate(ntpAutoUpdateToggle.checked);
    }

    // Initialize Light Schedule
    const lightScheduleToggle = document.getElementById('lightScheduleToggle');
    if (lightScheduleToggle) {
      toggleLightSchedule(lightScheduleToggle.checked);
    }

    // Initialize HomeAssistant Integration
    const haIntegrationToggle = document.getElementById('haIntegration');
    if (haIntegrationToggle) {
      toggleHaIntegration(haIntegrationToggle.checked);
    }
    
    // Initialize other toggles if present
    // Example:
    // const anotherToggle = document.getElementById('anotherToggleId');
    // if (anotherToggle) {
    //   toggleAnotherFeature(anotherToggle.checked);
    // }

  });