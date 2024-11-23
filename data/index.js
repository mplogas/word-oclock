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
    container.style.display = isChecked ? 'block' : 'none';
  }

  function toggleNtpTimeUpdate(isChecked) {
    const container = document.getElementById('ntpTimeUpdateContainer');
    container.style.display = isChecked ? 'block' : 'none';
  }

  function toggleNtpAutoUpdate(isChecked) {
    const container = document.getElementById('ntpAutoUpdateContainer');
    container.style.display = isChecked ? 'block' : 'none';
  }

  function toggleHaIntegration(isChecked) {
    const form = document.getElementById('haIntegrationContainer');
    form.style.display = isChecked ? 'block' : 'none';
  }

  function toggleAutoBrightness(isChecked) {
    const container = document.getElementById('autoBrightnessContainer');
    container.style.display = isChecked ? 'block' : 'none';
  }

  function toggleResetConfiguration(isChecked) {
    const container = document.getElementById('resetConfigurationContainer');
    container.style.display = isChecked ? 'block' : 'none';
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

    const autoBrightnessToggle = document.getElementById('autoBrightness');
    if (autoBrightnessToggle) {
      toggleAutoBrightness(autoBrightnessToggle.checked);
    }

    const resetConfigurationToggle = document.getElementById('resetConfiguration');
    if (resetConfigurationToggle) {
      toggleResetConfiguration(resetConfigurationToggle.checked);
    }
    
    // Initialize other toggles if present
    // Example:
    // const anotherToggle = document.getElementById('anotherToggleId');
    // if (anotherToggle) {
    //   toggleAnotherFeature(anotherToggle.checked);
    // }

    // Initialize Reset Configuration
    const resetButton = document.querySelector('.reset-button');
    if (resetButton) {
      resetButton.addEventListener('click', function(event) {
        const confirmation = confirm('Are you sure you want to reset the device? This action cannot be undone.');
        if (!confirmation) {
          event.preventDefault(); // Prevent form submission
        }
      });
    }

  });