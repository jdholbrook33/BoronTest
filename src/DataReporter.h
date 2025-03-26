#pragma once

#include "Particle.h"

class FlowSensor; // Forward declaration

class DataReporter {
public:
  DataReporter(FlowSensor* flowSensor, const char* deviceId);
  
  // Initialize reporter
  void begin();
  
  // Check and publish data as needed
  void update(unsigned long currentTime);
  
  // Publish flow data immediately
  void publishFlowData();
  
  // Publish diagnostic data immediately
  void publishDiagnosticData();
  
  // Getters
  unsigned long getLastPublishTime() const;
  unsigned long getLastDiagnosticPublishTime() const;
  
  // New method declaration
  unsigned long getValidTimestamp();
  
private:
  FlowSensor* _flowSensor;
  const char* _deviceId;
  String _firmwareVersion;
  
  unsigned long _lastPublishTime;
  unsigned long _lastDiagnosticPublishTime;
  
  // Reset reason tracking
  char _resetReasonStr[32];
  
  // Publishing intervals (in milliseconds)
  const unsigned long HOURLY_PUBLISH = 3600000;           // 1 hour (for demo)
  const unsigned long DIAGNOSTIC_PUBLISH_INTERVAL = 3600000; // 1 hour (heartbeat)
  // TODO: For production, adjust HOURLY_PUBLISH to 21600000 (6 hours) or 43200000 (12 hours)
  
  // Calculate hourly average water usage
  float calculateHourlyAverage() const;
  
  // Generate reset reason string
  void translateResetReason();
}; 