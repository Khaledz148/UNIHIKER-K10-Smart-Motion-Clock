# UNIHIKER K10 Smart Motion Clock

A stylish animated smart clock built with the **DFRobot UNIHIKER K10** using **Arduino IDE**.
It displays the current time, date, temperature, and humidity with a motion-reactive fragmented visual effect, WiFi setup portal, NTP time sync, and ambient-light-based RGB behavior.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Used](#hardware-used)
- [Software Used](#software-used)
- [Project Preview](#project-preview)
- [How It Works](#how-it-works)
- [Arduino IDE Setup](#arduino-ide-setup)
- [Required Libraries](#required-libraries)
- [Upload Instructions](#upload-instructions)
- [First-Time WiFi Setup](#first-time-wifi-setup)
- [Code Behavior](#code-behavior)
- [Configuration Notes](#configuration-notes)
- [Important Notes](#important-notes)
- [Useful Links](#useful-links)

---

## Overview

This project transforms the **UNIHIKER K10** into an interactive desk clock with a premium dark UI and animated fragmented digits.

The clock displays:
- Current time
- Current date and weekday
- Temperature
- Humidity

It also reacts to:
- Device movement using the onboard accelerometer
- Ambient light using the onboard light sensor

When the surrounding environment is dark, the RGB LEDs turn on with a smooth breathing effect. When the device is shaken or moved, the clock fragments shift dynamically and then settle back into place.

This project was developed using **Arduino IDE**.

---

## Features

- Large fragmented 7-segment animated clock
- Date and weekday display
- Temperature display from onboard AHT20 sensor
- Humidity display from onboard AHT20 sensor
- Motion-reactive visual effect using the accelerometer
- WiFi setup using **WiFiManager** captive portal
- Automatic NTP time synchronization
- Ambient-light-controlled RGB lighting
- Elegant dark-themed interface
- Configured for **Riyadh / Saudi Arabia timezone (UTC+3)**

---

## Hardware Used

- **DFRobot UNIHIKER K10**
- **USB Type-C cable**

---

## Software Used

- **Arduino IDE**
- **UNIHIKER K10 BSP**
- **WiFiManager library**

---

## Project Preview

### Main Features on Screen
- **Top bar:** current date and weekday
- **Center area:** large time display
- **Bottom left:** temperature
- **Bottom right:** humidity

![Main Screen](images/final-result.gif)

---

## How It Works

The project combines several onboard modules of the **UNIHIKER K10** to create an interactive smart clock experience.

### Main Logic Flow

1. On boot, the device initializes the screen, sensors, RGB LEDs, and WiFi.
2. If WiFi credentials are not saved, it opens a **WiFiManager captive portal**.
3. After connecting to WiFi, it synchronizes the current time using an **NTP server**.
4. The screen continuously updates:
   - Current time
   - Current date
   - Weekday
   - Temperature
   - Humidity
5. The accelerometer detects device movement and applies a fragmented motion effect to the clock digits.
6. The ambient light sensor monitors surrounding brightness:
   - In dark environments, RGB LEDs animate with a breathing effect.
   - In bright environments, the RGB LEDs remain off.

This gives the clock both a practical and decorative behavior.

---

## Arduino IDE Setup

To compile and upload this project successfully, make sure your Arduino IDE is properly prepared for the **UNIHIKER K10**.

### 1. Install Arduino IDE
Download and install the latest version of **Arduino IDE** from the official Arduino website.

### 2. Add UNIHIKER K10 Board Support Package
Open **File > Preferences**, then add the required board manager URL for the **UNIHIKER K10** package in the **Additional Boards Manager URLs** field.

After that:
- Open **Tools > Board > Boards Manager**
- Search for **UNIHIKER**
- Install the correct package for the K10 board

### 3. Select the Board
After installation, select **Board: UNIHIKER K10**. Also make sure the correct COM port is selected after connecting the device through USB.

---

## Required Libraries

Install the following library in Arduino IDE:
- **WiFiManager**

Depending on your environment and BSP version, the following are typically provided through the board package or core dependencies:
- WiFi
- WebServer
- DNSServer
- time / NTP-related functions
- sensor support for onboard modules

If any dependency is missing during compilation, install the required library through **Sketch > Include Library > Manage Libraries**.

---

## Upload Instructions

1. Connect the **UNIHIKER K10** to your computer using a **USB Type-C cable**.
2. Open the project `.ino` file in **Arduino IDE**.
3. Make sure the correct board and port are selected.
4. Click **Verify** to compile the code.
5. Click **Upload** to flash the project to the device.
6. Wait until the upload is completed successfully.

If upload fails, try:
- Checking the USB cable
- Reconnecting the device
- Selecting the correct COM port again
- Pressing reset on the board if needed

---

## First-Time WiFi Setup

On the first boot, or whenever WiFi credentials are not available, the device creates a temporary WiFi access point using **WiFiManager**.

### Steps

1. Power on the UNIHIKER K10.
2. Wait for the WiFi setup portal to appear.
3. On your phone or laptop, connect to the temporary WiFi network created by the device.
4. A captive portal page should open automatically. If it does not, open your browser and go to `192.168.4.1`.
5. Select your WiFi network and enter the password.
6. Save and connect.

---

## Code Behavior

### Time Display
The center of the screen shows the current time in a large fragmented style inspired by 7-segment displays.

### Fragment Animation
Each digit is visually divided into multiple segments. When motion is detected, the segments shift slightly to create a dynamic glitch-like effect. After movement stops, they smoothly return to position.

### Date and Weekday
The top section displays the current date and the day of the week.

### Temperature and Humidity
The onboard **AHT20 sensor** provides real-time temperature and humidity readings displayed at the bottom of the screen.

### Ambient Light Reaction
- **Dark environment:** RGB LEDs activate with a smooth breathing animation.
- **Bright environment:** RGB LEDs stay off.

### NTP Synchronization
Once WiFi is available, the device requests time from an NTP server and adjusts it for **Riyadh, Saudi Arabia (UTC+3)**.

---

## Configuration Notes

### Timezone
This project is configured for **UTC+3**. To change this, update the offset values in the code.

### NTP Server
You can replace the default server with `pool.ntp.org` or `time.nist.gov` if needed.

### Motion Sensitivity
If the animation is too sensitive, adjust the accelerometer threshold or motion smoothing values in the source code.

---

## Important Notes

- Requires **WiFi** for initial time synchronization.
- Saved credentials allow automatic reconnection after the first setup.
- Sensor values may fluctuate slightly depending on placement.
- Built specifically for the **UNIHIKER K10**; other ESP32 devices may require pinout modifications.

---

## Useful Links

- [DFRobot UNIHIKER](https://www.dfrobot.com/)
- [Arduino IDE](https://www.arduino.cc/en/software)
- [WiFiManager](https://github.com/tzapu/WiFiManager)
- [NTP Pool](https://www.ntppool.org/)
