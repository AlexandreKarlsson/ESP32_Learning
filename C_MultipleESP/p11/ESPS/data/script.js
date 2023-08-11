function onButton() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "on", true);
    xhttp.send();
}

function offButton() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "off", true);
    xhttp.send();
}

// Function to get luminosite data from the server
function getLuminosite() {
    var xhttp = new XMLHttpRequest();
  
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("valeurLuminosite").innerHTML = this.responseText;
      }
    };
  
    xhttp.open("GET", "lireLuminosite", true);
    xhttp.send();
  }
  
  // Function to get data from /esp1 endpoint
  function getEsp1Data() {
    var xhttp = new XMLHttpRequest();
  
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        // Update your HTML element with the received data
        document.getElementById("esp1Value").innerHTML = this.responseText;
      }
    };
  
    xhttp.open("GET", "esp1", true);
    xhttp.send();
  }
  
  // Function to get data from /esp2 endpoint
  function getEsp2Data() {
    var xhttp = new XMLHttpRequest();
  
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        // Update your HTML element with the received data
        document.getElementById("esp2Value").innerHTML = this.responseText;
      }
    };
  
    xhttp.open("GET", "esp2", true);
    xhttp.send();
  }
  
  // Call the functions every 2000 milliseconds (2 seconds)
  setInterval(getLuminosite, 2000);
  setInterval(getEsp1Data, 2000);
  setInterval(getEsp2Data, 2000);
  