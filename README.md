# ESP01_RCRX
A firmware for ESP01 (ESP8266) to control two servos, one servo and one DC motor or two DC motors.

![Demo](doc/demo.gif)

*A demonstration with two DC motors controlled with the Roboremo app on Android*

## Overview
This firmware is designed for the ESP01 module, based on the ESP8266 microcontroller. It supports controlling two channels which can be configured as servos or as H-bridge outputs for DC motors.

## Features
- Control via UDP: Receives commands over WiFi to adjust positions of the servos or motor speeds. Use,  e.g., (Roboremo Android app)[https://play.google.com/store/apps/details?id=com.hardcodedjoy.roboremofree] to build the controls you want.
- Supports Dual Mode: Each channel can independently be a servo or an H-bridge motor control.
- Remote Debugging: Optional remote debugging capabilities with visual feedback on connection and error states.

## Prerequisites
- Arduino IDE or compatible platform for ESP8266 development.
- ESP8266 board definitions and tools installed in the IDE.

## Hardware Setup
- **Channel 1**: Connect to GPIO 0 (Servo) or GPIO 0 and GPIO 2 (H-bridge motor 1).
- **Channel 2**: Connect to GPIO 1 (Servo) or GPIO 1/TX and GPIO 3/RX (H-bridge motor 2).
- Ensure the ESP01 module is powered adequately at 3.3V even during motor operation.
Do not connect ESP01 directly to the battery. Use a 3.3V regulator.

## Configuration
1. Define `WIFI_SSID` and `WIFI_PWD` in the code to match your network credentials.
2. (Optional) Configure the module as an Access Point (uncomment `#define USE_AP` and adjust settings as needed).

## Installation
1. Clone this repository.
2. Open the project in the Arduino IDE.
3. Select the correct board from the Tools > Board menu (ESP8266).
4. Compile and upload the firmware to your ESP01.

## Usage
Send UDP packets to the device's IP address at port 4242. The packet content should command the servo or motor:
- For servos: Send `s <value>` or `t <value>` to set positions of channel 1 and 2 respectively.
- For motors: Send `b <value1> <value2>` to set speeds of both channels.

## Debugging
Enable `#define REMOTE_DEBUG` to use Telnet for real-time debugging and monitoring of the system state.

## Contributing
Contributions to the project are welcome. Please fork the repository and submit pull requests with your enhancements.

## License
This project is released under the MIT license.
