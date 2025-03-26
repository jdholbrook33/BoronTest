# Pool Fill Monitor

## Overview
The Pool Fill Monitor is an ESP32-based display system that receives data from a BORON IoT device monitoring a swimming pool filler. The system displays flow rate, total gallons, signal strength, and provides a real-time scrolling graph of flow history.

## Hardware
- ESP32-2432S028R (CYD - "Cheap Yellow Display")
- BORON IoT device with flow sensor (data source)
- Optional: Micro SD card for logging

## Features
- Real-time display of flow data (GPM)
- Total gallons counter
- Signal strength indicator with visual bar
- Scrolling 60-minute flow history graph
- SD card logging capability
- Touch interface for switching between views
- JSON data parsing from serial input

## Screens
The system has two main screens:
1. **Stats Display** - Shows current flow rate, total gallons, signal strength, and last update time
2. **Graph Display** - Shows a scrolling 60-minute history of flow rate with automatic scaling

## JSON Data Format
The ESP32 expects JSON data from the BORON device in the following format:

```json
{
  "gpm": 2.5,       // Current flow rate in gallons per minute (float)
  "gallons": 125.5, // Total gallons (float)
  "signal": 85,     // Signal strength percentage (integer, 0-100)
  "time": "12:34:56" // Timestamp (string)
}
```

## Setup Instructions

### ESP32 Setup
1. Load the provided code onto your ESP32-2432S028R board using PlatformIO or Arduino IDE
2. Connect the ESP32 to your BORON device via serial connection (RX/TX)
3. Power on the ESP32

### BORON Device Configuration
1. Program your BORON device to collect flow data from your pool filler sensor
2. Format the data in the JSON format shown above
3. Send the JSON string over serial to the ESP32 at 115200 baud rate
4. For testing, you can send simulated data from a computer to verify the display functions

## Interface Guide
- **MODE Button**: Toggle between Stats and Graph views
- **LOG Button**: Toggle SD card logging (only appears if SD card is detected)

## Logging
When SD card logging is enabled:
- All received data is logged to `/terminal_log.txt` on the SD card
- The LOG button turns green when logging is active
- Log entries include raw JSON data and any parsing errors

## Hardware Connections
Refer to the pin definitions in the code and "CYD pin assignments.txt" for the specific pin connections.

## Customization
The code can be modified to:
- Change display colors and layout
- Adjust graph scaling
- Modify data collection parameters
- Add additional sensors or data fields

## Troubleshooting
- If no data appears, check serial connections and baud rate
- Verify JSON format matches the expected structure
- Check SD card if logging fails
- Signal strength below 30% may indicate connection issues

## Project Structure
- `main.cpp`: Main application code
- `platformio.ini`: Project configuration for PlatformIO
- Libraries:
  - TFT_eSPI: Display driver
  - XPT2046_Touchscreen: Touchscreen driver
  - ArduinoJson: JSON parsing
  - SPI, SD, FS: For SD card functionality

## Credits
This project was developed as a custom solution for monitoring pool fill levels using readily available ESP32 display hardware and BORON IoT devices. 