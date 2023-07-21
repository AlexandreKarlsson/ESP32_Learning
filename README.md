### ESP32_Learning
This repository has been created to learn how to use the ESP32. The ultimate goal is to enable communication between 2 or more ESP32s over WiFi using protobuf serialization and visualize the data on a web page. The project is divided into different tasks, and the folders in this project are organized accordingly. The folder names consist of 3 letters representing the number of ESPs involved. Inside these folders, you can find projects starting with a number that represents the path to the solution.

## Description of the Paths
Let's take a look at the project paths:

1. Connecting an ESP32 to WiFi *(PC <=> ESP32)*
2. Processing UART commands *(PC <=> ESP32)*
3. Enabling communication between 2 ESP32 boards *(PC <=> ESP32 <=> ESP32)*
4. The same but with protobuf communication through the Nanopb library *(PC <=> ESP32 <=> ESP32)*
5. Using WiFiManager to connect automatically to WiFi *(ESP32)*
6. Automatically connect all ESP32s through WiFiManager using ESPNOW *(ESP32 <=> ESP32)*
7. Launching and interacting through a web page hosted on the ESP32 *(ESP32)*
8. Project: An ESP server connects to WiFi with WiFiManager, ESP clients ask for the WiFi information, and then connect to the ESP server through it to send information using protobuf.
9. The same as Project 8, but the ESP server hosts a web page at the same time.

## Dependencies
### WiFiManager
One of the easiest ways to connect the ESP32 to a WiFi network is to use WiFiManager, which will automatically set up a network for you and give the ESP32 the SSID and password for your router.
### Nanopd
Nanopd is a library designed to use protobuf in embedded systems.

## 1 : Connecting WiFi by Tomasz Tarnowski
The first project simply makes the ESP32's built-in LED flash based on the WiFi connection status. This project is a copy of Tomasz Tarnowski's code, which can be found in this video: https://www.youtube.com/watch?v=aAG0bp0Q-y4&t=171s&ab_channel=TomaszTarnowski. We are using the non-blocking approach.

### How it works
1. The LED is ON when the ESP32 is connected to WiFi.
2. The LED flashes when waiting for a connection.

## 2 : Receiving a UART command
This second project causes the ESP32 to blink with a delay set by the user via the serial monitor using the UART. For example, "blink 200" will cause the ESP32 to blink for 200 milliseconds. The ESP32 waits for a WiFi connection to process UART commands.

### How it works
1. The LED works as in the first WiFi connection status project.
2. The LED flashes once when a serial signal is received.
3. The LED flashes twice when a flashing command is detected.
4. The LED flashes three times when a flash command is acknowledged before execution.

## 3 : ESP connection
These two projects allow communication between two ESP32 boards. It uses a client-server connection over WiFi, where ESPC is the client and ESPS is the server. To simplify the process, you must first connect ESPS and obtain its IP address (using your computer's share point can make it easier to obtain this address). Then enter this IP address in the ESPC code before compiling. Once both ESP32 boards are connected to WiFi, you can issue a blink command to ESPC via the UART and it will be executed by ESP2.

### How it works - ESPC
1. WiFi connection, same as the first project.
2. User enters "blink x", where x is an integer representing the delay.
3. The LED blinks three times before sending the UART command over the WiFi connection.
4. The serial monitor will indicate when an ACK is received and will wait for it. A "." will appear at the frequency of receiving messages from the server with a newline character ("\n").
### How it works - ESPS
1. The LED flashes ten times to confirm that it is the ESP32 server after compilation (or when the reset button is pressed).
2. WiFi connection, same as the first project.
3. If it's a flash command, the LED blinks three times before the flash command is executed.
4. The LED will still flash three times to indicate that it has received a signal.

*Note: Please be sure to replace the placeholder values in the code with your specific configurations and follow the instructions provided in the respective folders for each project.*

## 4 : ESP Connection using Nanopb
Same as project 3 but using nanopb.
### How it works - ESPC
A sendCommand will encode the message trough a structure define in message.pb.h (obtained with a message.proto)
### How it works - ESPS
A processCommand will decode the message.

## 5 : WiFiManager
Simple use of wifimanager

## 6 : Auto pairing
The ESP32 client needs to have the MAC address of the ESP server to launch communication with ESPNOW. They will request wifi information that the ESP server has (or will obtain with WiFiManager).

## 7 : Launching a web page
Credit goes to Thomas Desrocher: https://www.youtube.com/redirect?event=video_description&redir_token=QUFFLUhqbWZEZjh1MHVDNlhFb1JCTkRsbDE5NmI3OFd3d3xBQ3Jtc0tuNXhsUjJnbklYdUZlVWlRelJhQkJfTmxzQnFUVlhWTUZackIzV3RqSnFNenpmT1k1bkpwNHlDdVZPc1hQM25xX3FEeFpROVc3SGxqT3RrQVFEOE4teWpXeWxwM2tsOXlCWF94OWpVRVdyVG5GN0V6TQ&q=https%3A%2F%2Ftommydesrochers.com%2Fla-meilleure-methode-pour-heberger-une-interface-web-sur-esp32-version-avancee-esp32-ep4%2F&v=DDMQbqPUliY

## 8 : ESP Connection using Nanopb
First project: ESP server will auto-pair the ESP clients, and they will exchange communication in WiFi using protobuf.
### How it works - ESPC
Clients will ask wifi information with ESPNOW. When connected to WiFi, they will send to the server IP a protobuf message containing an analog reading value of a touch port.
### How it works - ESPS
Server will connect to WiFi using WiFiManager and then share the information with all the ESPNOW incoming clients. The server will print in the monitor all the value that it received.

## 9 : ESP Connection using Nanopb and with a web page
Second project: ESP server will host at the same time a web page.
### How it works - ESPC
Clients will ask wifi information with ESPNOW. When connected to WiFi, they will


*WARNING SECURITY : the security of your network is engaged because the wifi information are share with all clients comming trough ESPNOW.