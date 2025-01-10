//light.html
// Function to toggle the light status
function toggleLight(isChecked) {
  const status = isChecked ? '1' : '0';

  fetch(`/toggleLight?enabled=${status}`)
    .then(response => response.text())
    .then(data => {
      console.log(`Light toggled: ${data}`);
    })
    .catch(error => console.error('Error:', error));
}

// Function to set the light color
function selectLightColor(color) {
  // Validate color format (e.g., #RRGGBB)
  const colorRegex = /^#[0-9A-Fa-f]{6}$/;
  if (!colorRegex.test(color)) {
    alert('Invalid color format.');
    return;
  }

  fetch(`/setLightColor?color=${encodeURIComponent(color)}`)
    .then(response => response.text())
    .then(data => {
      console.log(`Color set: ${data}`);
    })
    .catch(error => console.error('Error:', error));
}

// Function to handle auto-brightness toggle
function toggleAutoBrightness(isEnabled) {
  const brightnessSliderContainer = document.getElementById('brightnessSliderContainer');
  brightnessSliderContainer.style.display = isEnabled ? 'none' : 'block';

  const enabled = isEnabled ? '1' : '0';

  fetch(`/setAutoBrightness?enabled=${enabled}`)
    .then(response => response.text())
    .then(data => {
      console.log(`Auto-brightness toggled: ${data}`);
    })
    .catch(error => console.error('Error:', error));
}

// Function to set brightness level
function setBrightness(value) {
  // Validate brightness value (should be between 0 and 255)
  const brightness = parseInt(value, 10);
  if (isNaN(brightness) || brightness < 0 || brightness > 255) {
    alert('Invalid brightness value.');
    return;
  }

  fetch(`/setBrightness?value=${brightness}`)
    .then(response => response.text())
    .then(data => {
      console.log(`Brightness set: ${data}`);
    })
    .catch(error => console.error('Error:', error));
}

// Function to update the displayed brightness value
function updateBrightnessValue(value) {
  document.getElementById('brightnessValue').innerText = value;
}

// time.html
function saveTimezone() {
  const timezone = document.getElementById('timezoneSelect').value;
  const formData = new FormData();
  formData.append('timezone', timezone);

  fetch('/saveTimezone', {
      method: 'POST',
      body: formData
  })
      .then(response => response.text())
      .then(data => {
          console.log(`Timezone saved: ${data}`);
      })
      .catch(error => console.error('Error saving timezone:', error));
}

function updateCurrentTime() {
  fetch('/getCurrentTime')
      .then(response => response.json())
      .then(data => {
          document.getElementById('currentTime').innerText = data.time;
      })
      .catch(error => console.error('Error fetching current time:', error));
}

// Call updateCurrentTime every 30 seconds
//setInterval(updateCurrentTime, 30000);

// Call it once immediately to set the initial time
//updateCurrentTime();

// system.html
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
  // const form = document.getElementById('haIntegrationContainer');
  // form.style.display = isChecked ? 'block' : 'none';
}

function toggleResetConfiguration(isChecked) {
  const container = document.getElementById('resetConfigurationContainer');
  container.style.display = isChecked ? 'block' : 'none';
}

// Function to handle form submission
function saveHaIntegration() {
  const haIntegrationToggle = document.getElementById('haIntegrationToggle').checked;
  const mqttHost = document.getElementById('brokerIP').value;
  const mqttPort = document.getElementById('brokerPort').value;
  const mqttUsername = document.getElementById('mqttUsername').value;
  const mqttPassword = document.getElementById('mqttPassword').value;
  const mqttTopic = document.getElementById('defaultTopic').value;

  if (!mqttHost || !mqttPort || !mqttTopic) {
    alert('Please fill in all required fields.');
    return;
  }

  const formData = new FormData();
  formData.append('enabled', haIntegrationToggle ? '1' : '0');
  if(haIntegrationToggle) {
    formData.append('mqttHost', mqttHost);
    formData.append('mqttPort', mqttPort);
    if(mqttUsername.length > 0) {
      formData.append('mqttUsername', mqttUsername);
      if(mqttPassword.length > 0) {
        formData.append('mqttPassword', mqttPassword);
      }
    }
    if(mqttTopic.length > 0) formData.append('mqttTopic', mqttTopic);
  }


  fetch('/setHaIntegration', {
    method: 'POST',
    body: formData
  })
    .then(response => response.text())
    .then(data => {
      console.log(`HomeAssistant integration saved: ${data}`);
    })
    .catch(error => console.error('Error:', error));
}

function resetConfiguration(event) { 
  const confirmation = confirm('Are you sure you want to reset the device? This action cannot be undone.');
    if (!confirmation) {
      event.preventDefault(); // Prevent form submission
    } else {
      fetch('/resetConfig',{
        method: 'POST',
        body: ""
      })
        .then(response => response.text())
        .then(data => {
          console.log(`Configuration reset: ${data}`);
        })
        .catch(error => console.error('Error:', error));
    }
}


// Theme
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

// Initializer

document.addEventListener('DOMContentLoaded', function () {
  // Light Page
  const autoBrightnessToggle = document.getElementById('autoBrightnessToggle');
  if (autoBrightnessToggle) {
    toggleAutoBrightness(autoBrightnessToggle.checked);
  }


  // System Page
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
  const haIntegrationToggle = document.getElementById('haIntegrationToggle');
  if (haIntegrationToggle) {
    toggleHaIntegration(haIntegrationToggle.checked);
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
    resetButton.addEventListener('click', resetConfiguration);
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