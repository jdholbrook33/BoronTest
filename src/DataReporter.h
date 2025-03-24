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
  
  // Publishing intervals
  const unsigned long HOURLY_PUBLISH = 600000;           // 10 minutes for testing
  const unsigned long DIAGNOSTIC_PUBLISH_INTERVAL = 900000; // 15 minutes for testing
  
  // Calculate hourly average water usage
  float calculateHourlyAverage() const;
  
  // Generate reset reason string
  void translateResetReason();
}; 