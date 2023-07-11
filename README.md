### ESP32_Learning
This repository has been created to learn how to use the ESP32. The ultimate goal is to enable communication between 2 or more ESP32s over WiFi using protobuf serialisation. A command sent via UART to one ESP32 will perform tasks or make requests to the others. The problem is divided into different tasks, and the folders in this project are organised accordingly, starting with a number that represents the path to the solution.

## Description of the Paths:
- Connecting an ESP32 to WiFi (PC <=> ESP32)
- Process UART commands (PC <=> ESP32)
- Enable communication between 2 ESP32 boards (PC <=> ESP32 <=> ESP32)

## Dependencies
One of the easiest ways to connect the ESP32 to a WiFi network is to use WiFiManager, which will automatically set up a network for you and give the ESP32 the SSID and password for your router. Unfortunately, I was unable to get it to work with this project using PlatformIO from VS Code. However, WiFiManager works with the Arduino IDE and is easy to implement.

## Connecting WiFi by Tomasz Tarnowski
The first project simply makes the ESP32's built-in LED flash based on the WiFi connection status. This project is a copy of Tomasz Tarnowski's code, which can be found in this video: YouTube link. We are using the non-blocking approach.

### How it works
1. The LED is ON when the ESP32 is connected to WiFi.
2. The LED flashes when waiting for a connection.

## Receiving a UART command
This second project causes the ESP32 to blink with a delay set by the user via the serial monitor using the UART. For example, "blink 200" will cause the ESP32 to blink for 200 milliseconds. The ESP32 waits for a WiFi connection to process UART commands.

### How it works
1. The LED works as in the first WiFi connection status project.
2. The LED flashes once when a serial signal is received.
3. The LED flashes twice when a flashing command is detected.
4. The LED flashes three times when a flash command is acknowledged before execution.
## ESP connection
These two projects allow communication between two ESP32 boards. It uses a client-server connection over WiFi, where ESP1 is the client and ESP2 is the server. To simplify the process, you must first connect ESP2 and obtain its IP address (using your computer's share point can make it easier to obtain this address). Then enter this IP address in the ESP1 code before compiling. Once both ESP32 boards are connected to WiFi, you can issue a blink command to ESP1 via the UART and it will be executed by ESP2.

### How it works - ESP1
1. WiFi connection, same as first project.
2. User enters "blink x", where x is an integer representing the delay.
3. The LED blinks three times before sending the UART command over the WiFi connection.
4. The serial monitor will indicate when an ACK is received and will wait for it. A "." will appear at the frequency of receiving messages from the server with a newline character ("\n").
### How it works - ESP2
1. The LED flashes ten times to confirm that it is the ESP32 server after compilation (or when the reset button is pressed).
2. WiFi connection, same as the first project.
3. If it's a flash command, the LED blinks three times before the flash command is executed.
4. The LED will still flash three times to indicate that it has received a signal.
*Note: Please be sure to replace the placeholder values in the code with your specific configurations and follow the instructions provided in the respective folders for each project.