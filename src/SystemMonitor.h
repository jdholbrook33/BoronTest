#pragma once

#include "Particle.h"

class SystemMonitor {
public:
  SystemMonitor();
  
  // Initialize system monitoring
  void begin();
  
  // Update routine
  void update();
  
  // Check if boot sequence is complete
  bool isBootComplete() const;
  
  // Get watchdog reset count
  int getWatchdogResetCount() const;

private:
  // Boot sequence tracking
  bool _bootSequenceComplete;
  unsigned long _bootStartTime;
  
  // Watchdog tracking
  int _watchdogResetCount;
  ApplicationWatchdog *_watchdog;
  
  // Boot sequence parameters
  const unsigned long BOOT_DELAY = 30000;     // 30 second boot delay
  const unsigned long CONNECTION_WAIT = 60000; // 60 second connection timeout
  const unsigned long WATCHDOG_TIMEOUT = 60000; // 60 second watchdog timeout
  
  // Static callback for watchdog
  static void watchdogHandler();
  
  // Helper functions
  void initializeWatchdog();
  void checkBootSequence(unsigned long currentTime);
}; 