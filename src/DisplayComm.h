#pragma once

#include "Particle.h"
#include "FlowSensor.h"

class DisplayComm {
public:
  DisplayComm(FlowSensor* flowSensor);
  
  // Initialize UART and other settings
  void begin();
  
  // Update method to be called in main loop
  void update(unsigned long currentTime);
  
  // Manually send data to display
  void sendDisplayData();
  
private:
  // Component references
  FlowSensor* _flowSensor;
  
  // Last update tracking
  unsigned long _lastDisplayUpdateTime;
  
  // UART initialization
  void initializeUART();
  
  // Format JSON data to send to display
  String formatDisplayData();
  
  // Update interval in milliseconds (5 seconds for testing)
  const unsigned long DISPLAY_UPDATE_INTERVAL = 5000;
}; 