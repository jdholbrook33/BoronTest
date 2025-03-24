#include "DataReporter.h"
#include "FlowSensor.h"

DataReporter::DataReporter(FlowSensor* flowSensor, const char* deviceId) :
  _flowSensor(flowSensor),
  _deviceId(deviceId),
  _firmwareVersion("1.0.0"),
  _lastPublishTime(0),
  _lastDiagnosticPublishTime(0)
{
  memset(_resetReasonStr, 0, sizeof(_resetReasonStr));
  strcpy(_resetReasonStr, "Unknown");
}

void DataReporter::begin() {
  // Translate reset reason for initial diagnostics
  translateResetReason();
  
  // Initialize publish times to avoid immediate publishing
  _lastPublishTime = millis();
  _lastDiagnosticPublishTime = millis();
  
  Serial.println("Data reporter initialized");
}

void DataReporter::update(unsigned long currentTime) {
  // Check if it's time for regular flow data publish
  if (currentTime - _lastPublishTime >= HOURLY_PUBLISH) {
    publishFlowData();
  }
  
  // Check if it's time for diagnostic publish
  if (currentTime - _lastDiagnosticPublishTime >= DIAGNOSTIC_PUBLISH_INTERVAL) {
    publishDiagnosticData();
  }
}

void DataReporter::publishFlowData() {
  // Calculate hourly average
  float hourlyAverage = calculateHourlyAverage();
  
  // Get accumulated gallons and convert to whole number for customer display
  float accumulatedGallons = _flowSensor->getAccumulatedGallons();
  int wholeGallons = (int)accumulatedGallons;
  
  // Get current timestamp, ensure it's valid
  unsigned long timestamp = getValidTimestamp();
  
  // Create JSON payload
  char jsonBuffer[256];
  snprintf(jsonBuffer, sizeof(jsonBuffer), 
           "{\"device_id\":\"%s\",\"timestamp\":%lu,\"gallons_used\":%.0f,\"hourly_average\":%.1f}",
           _deviceId, timestamp, (float)wholeGallons, hourlyAverage);
  
  Serial.printlnf("Publishing flow data: %s", jsonBuffer);
  Particle.publish("flow_data", jsonBuffer, PRIVATE);
  
  // Log the publish event
  Serial.printlnf("Flow publish: %d gallons this interval, %.1f gallons average per hour", 
                 wholeGallons, hourlyAverage);
  Serial.printlnf("Daily total: %.2f gallons over %d hours", 
                 _flowSensor->getDailyGallonsTotal(), _flowSensor->getHoursElapsed());
  
  // Reset accumulated gallons for next interval
  _flowSensor->resetAccumulatedGallons();
  
  // Increment hours elapsed
  _flowSensor->incrementHoursElapsed();
  
  // Update last publish time
  _lastPublishTime = millis();
}

void DataReporter::publishDiagnosticData() {
  // Update reset reason
  translateResetReason();
  
  // Get cellular signal strength
  CellularSignal sig = Cellular.RSSI();
  int signalStrength = sig.getStrength();
  
  // Get current timestamp, ensure it's valid
  unsigned long timestamp = getValidTimestamp();
  
  // Create JSON payload with detailed device info
  char jsonBuffer[256];
  snprintf(jsonBuffer, sizeof(jsonBuffer), 
           "{\"device_id\":\"%s\",\"timestamp\":%lu,\"firmware\":\"%s\",\"reset_reason\":\"%s\","
           "\"lifetime_gallons\":%.2f,\"daily_total\":%.2f,\"hours_elapsed\":%d,\"total_pulses\":%lu,"
           "\"flow_events_today\":%d,\"signal_strength\":%d}",
           _deviceId, timestamp, _firmwareVersion.c_str(), _resetReasonStr, 
           _flowSensor->getLifetimeGallons(), _flowSensor->getDailyGallonsTotal(), 
           _flowSensor->getHoursElapsed(), _flowSensor->getTechnicalPulseCount(), 
           _flowSensor->getFlowEventsToday(), signalStrength);
  
  Serial.printlnf("Publishing diagnostic data: %s", jsonBuffer);
  bool success = Particle.publish("diagnostic_data", jsonBuffer, PRIVATE);
  
  if (success) {
    Serial.printlnf("Diagnostic data published successfully");
  } else {
    Serial.printlnf("Failed to publish diagnostic data");
  }
  
  // Update last diagnostic publish time
  _lastDiagnosticPublishTime = millis();
}

float DataReporter::calculateHourlyAverage() const {
  float hourlyAverage = 0.0;
  int hoursElapsed = _flowSensor->getHoursElapsed();
  
  if (hoursElapsed > 0) {
    hourlyAverage = _flowSensor->getDailyGallonsTotal() / hoursElapsed;
  }
  
  return hourlyAverage;
}

void DataReporter::translateResetReason() {
  int reason = System.resetReason();
  
  switch (reason) {
    case RESET_REASON_PIN_RESET:
      strcpy(_resetReasonStr, "Pin Reset");
      break;
    case RESET_REASON_POWER_MANAGEMENT:
      strcpy(_resetReasonStr, "Power Management");
      break;
    case RESET_REASON_WATCHDOG:
      strcpy(_resetReasonStr, "Watchdog Timer");
      break;
    case RESET_REASON_UPDATE:
      strcpy(_resetReasonStr, "Firmware Update");
      break;
    case RESET_REASON_UPDATE_TIMEOUT:
      strcpy(_resetReasonStr, "Firmware Update Timeout");
      break;
    case RESET_REASON_FACTORY_RESET:
      strcpy(_resetReasonStr, "Factory Reset");
      break;
    case RESET_REASON_SAFE_MODE:
      strcpy(_resetReasonStr, "Safe Mode");
      break;
    case RESET_REASON_DFU_MODE:
      strcpy(_resetReasonStr, "DFU Mode");
      break;
    case RESET_REASON_PANIC:
      strcpy(_resetReasonStr, "System Panic");
      break;
    case RESET_REASON_USER:
      strcpy(_resetReasonStr, "User Requested");
      break;
    case RESET_REASON_UNKNOWN:
    default:
      strcpy(_resetReasonStr, "Unknown");
      break;
  }
  
  Serial.printlnf("Reset reason: %s (code: %d)", _resetReasonStr, reason);
}

unsigned long DataReporter::getLastPublishTime() const {
  return _lastPublishTime;
}

unsigned long DataReporter::getLastDiagnosticPublishTime() const {
  return _lastDiagnosticPublishTime;
}

// Helper method to get a valid timestamp
unsigned long DataReporter::getValidTimestamp() {
  // First try to get the current time
  if (Time.isValid()) {
    return Time.now();
  }
  
  // If time sync hasn't happened yet, try to sync
  Particle.syncTime();
  
  // Wait a bit for the sync to complete (max 3 seconds)
  unsigned long start = millis();
  while (!Time.isValid() && millis() - start < 3000) {
    Particle.process();
    delay(100);
  }
  
  // Return the time or a fallback if still not valid
  if (Time.isValid()) {
    Serial.println("Time synchronized successfully");
    return Time.now();
  } else {
    // If we can't get a valid time, use millis/1000 + a base time
    // This isn't accurate but better than using an obviously wrong time
    Serial.println("Warning: Using fallback timestamp, time sync failed");
    return millis()/1000 + 1711302000; // March 24, 2024 as base
  }
} 