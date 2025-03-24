# Boron Flow Meter - Diagnostic Channel

## Overview
This document describes the implementation of a diagnostic data channel for the Boron-based IoT flow meter system. The diagnostic channel provides essential system health and performance metrics that are separate from the customer-facing flow data.

## Purpose
The diagnostic channel allows system operators to monitor device health and troubleshoot issues without cluttering the customer-facing data stream. This separation of concerns helps maintain clean data for end users while still providing technical staff with necessary monitoring capabilities.

## Implementation Details

### Diagnostic Data Points
The diagnostic channel currently includes these key metrics:

1. **Watchdog Reset Count**: Tracks how many times the device has been reset by the watchdog timer, indicating potential system hangs or issues.
2. **Reset Reason**: Provides a human-readable explanation of why the system last restarted (e.g., watchdog, power cycle, firmware update).
3. **Lifetime Gallons**: Tracks the total gallons measured by the device throughout its operational life.
4. **Firmware Version**: Identifies the current firmware running on the device.

### Publishing Details
- **Topic Name**: `diagnostic_data`
- **Publish Interval**: Every 15 minutes (configurable via `DIAGNOSTIC_PUBLISH_INTERVAL`)
- **Data Format**: JSON payload
- **Visibility**: PRIVATE (only accessible via Particle cloud)

### JSON Payload Format
```json
{
  "device_id": "pool_1",
  "firmware": "1.0.0",
  "reset_count": 0,
  "reset_reason": "Power Management",
  "lifetime_gallons": 12345.67
}
```

### Debug Output
The implementation includes Serial debug messages to verify what's being published:
- Displays JSON payload before publishing
- Confirms successful/failed publishing attempts
- Shows reset reason translation from numeric code to string

## Future Expansion
The diagnostic channel is designed for easy expansion. Future metrics that could be added include:

1. **System Information**:
   - System uptime
   - Free memory

2. **Connection Statistics**:
   - Cellular signal strength
   - Connection quality metrics
   - Connection failures count

3. **Flow Meter Diagnostics**:
   - Total pulse count
   - Average pulses per second

4. **Publish/Cloud Metrics**:
   - Failed publish attempts
   - Publish latency

## Usage
The diagnostic data can be accessed via:
1. Particle Console - Subscribe to the `diagnostic_data` event
2. Particle Webhooks - Forward to your own server/dashboard
3. Particle Integrations - Send to data platforms like Google Cloud, Azure, etc.

No action is required from end users for this feature - it operates in the background to support system maintenance and monitoring.