#pragma once

#include "Particle.h"

class FlowSensor {
public:
  FlowSensor(int sensorPin, int ledPin, float pulsesPerGallon);
  
  // Initialization
  void begin();
  
  // To be called in main loop
  void update();
  
  // Check and handle flow events
  void checkFlow(unsigned long currentTime);
  
  // Reset daily counters
  void performDailyReset();
  
  // Getters for various counters
  float getAccumulatedGallons() const;
  float getDailyGallonsTotal() const;
  float getLifetimeGallons() const;
  int getFlowEventsToday() const;
  unsigned long getCustomerPulseCount() const;
  unsigned long getTechnicalPulseCount() const;
  bool isFlowActive() const;
  void resetAccumulatedGallons();
  int getHoursElapsed() const;
  void incrementHoursElapsed();
  
  // Static pulse counter for interrupt
  static void pulseCounterStatic();
  
private:
  // Configuration
  int _sensorPin;
  int _ledPin;
  float _pulsesPerGallon;
  float _gallonsPerPulse; // Pre-calculated inverse for optimization
  
  // Pulse counting
  volatile static unsigned long _customerPulseCount;
  volatile static unsigned long _technicalPulseCount;
  volatile static bool _pulseDetected;
  unsigned long _lastCustomerPulseCount;
  unsigned long _lastTechnicalPulseCount;
  
  // Flow tracking
  bool _flowActive;
  unsigned long _flowStartPulse;
  unsigned long _inactivityTimer;
  unsigned long _lastCheckTime;
  
  // Counters
  float _accumulatedGallons;
  float _lifetimeGallons;
  float _dailyGallonsTotal;
  int _flowEventsToday;
  int _hoursElapsed;
  
  // Constants
  const unsigned long FLOW_CHECK_INTERVAL = 5000;  // 5 seconds
  const unsigned long FLOW_TIMEOUT = 20000;        // 20 seconds
  const unsigned int MIN_PULSE_THRESHOLD = 5;      // Min pulses to consider flow active
  const float MIN_GALLONS_THRESHOLD = 0.05;        // Min gallons to record
  
  // Helper methods
  void handleFlowStart(unsigned long newCustomerPulses);
  void handleFlowEnd(unsigned long currentTime);
  void updateLedStatus();
  
  // Static instance reference for interrupt handling
  static FlowSensor* _instance;
}; 