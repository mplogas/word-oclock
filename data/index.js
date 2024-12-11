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

  // function toggleAutoBrightness(isChecked) {
  //   const container = document.getElementById('autoBrightnessContainer');
  //   container.style.display = isChecked ? 'block' : 'none';
  // }

  // Function to handle auto-brightness toggle
function toggleAutoBrightness(isEnabled) {
  const brightnessSliderContainer = document.getElementById('brightnessSliderContainer');
  if (isEnabled) {
      brightnessSliderContainer.style.display = 'none';
      // Send request to enable auto-brightness
      // fetch('/autoBrightness?enabled=1')
      //     .then(response => {
      //         if (response.ok) {
      //             console.log('Auto-brightness enabled');
      //         }
      //     });
  } else {
      brightnessSliderContainer.style.display = 'flex';
      // Send request to disable auto-brightness
      // fetch('/autoBrightness?enabled=0')
      //     .then(response => {
      //         if (response.ok) {
      //             console.log('Auto-brightness disabled');
      //         }
      //     });
  }
}

// Function to update brightness value display
function updateBrightnessValue(value) {
  document.getElementById('brightnessValue').innerText = value;
}

function setBrightness(value) {
  // Send request to set brightness value
  // fetch(`/setBrightness?value=${value}`)
  //     .then(response => {
  //         if (response.ok) {
  //             console.log(`Brightness set to ${value}`);
  //         }
  //     });
}

  function toggleResetConfiguration(isChecked) {
    const container = document.getElementById('resetConfigurationContainer');
    container.style.display = isChecked ? 'block' : 'none';
  }

  // Apply theme based on user's preference
  function applyTheme(theme) {
    if (theme === 'dark') {
      document.body.classList.add('dark-theme');
    } else {
      document.body.classList.remove('dark-theme');
    }
  }

  // Toggle the theme and save preference
  function toggleTheme() {
    const isDark = document.getElementById('theme-toggle').checked;
    const theme = isDark ? 'dark' : 'light';
    applyTheme(theme);
    localStorage.setItem('theme', theme);
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

    const resetConfigurationToggle = document.getElementById('resetConfiguration');
    if (resetConfigurationToggle) {
      toggleResetConfiguration(resetConfigurationToggle.checked);
    }

    const autoBrightnessToggle = document.getElementById('autoBrightness');
    if (autoBrightnessToggle) {
      toggleAutoBrightness(autoBrightnessToggle.checked);
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

    const toggleSwitch = document.getElementById('theme-toggle');
    const currentTheme = localStorage.getItem('theme') || 'light';
    // Set initial theme
    applyTheme(currentTheme);
    // Set toggle switch state
    toggleSwitch.checked = currentTheme === 'dark';
    // Add event listener
    toggleSwitch.addEventListener('change', toggleTheme);

  });