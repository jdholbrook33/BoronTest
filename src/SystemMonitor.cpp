#include "SystemMonitor.h"
#include "Storage.h"

SystemMonitor::SystemMonitor() :
  _bootSequenceComplete(false),
  _bootStartTime(0),
  _watchdogResetCount(0),
  _watchdog(nullptr)
{
}

void SystemMonitor::begin() {
  // Record boot start time
  _bootStartTime = millis();
  _bootSequenceComplete = false;

  // Load watchdog reset count from storage
  _watchdogResetCount = Storage::loadWatchdogResetCount();

  // Check if last reset was due to watchdog
  if (System.resetReason() == RESET_REASON_WATCHDOG) {
    _watchdogResetCount++;
    Storage::saveWatchdogResetCount(_watchdogResetCount);
    Serial.printlnf("Watchdog reset detected! Total count: %d", _watchdogResetCount);
  }

  // Initialize the watchdog
  initializeWatchdog();
  
  Serial.println("Starting boot sequence...");
  Serial.println("Requesting time sync from Particle Cloud...");
  Particle.syncTime();
}

void SystemMonitor::update() {
  // Check in with watchdog to prevent reset
  if (_watchdog != nullptr) {
    _watchdog->checkin();
  }
  
  // Check boot sequence completion if not already complete
  if (!_bootSequenceComplete) {
    checkBootSequence(millis());
  }
}

bool SystemMonitor::isBootComplete() const {
  return _bootSequenceComplete;
}

int SystemMonitor::getWatchdogResetCount() const {
  return _watchdogResetCount;
}

void SystemMonitor::watchdogHandler() {
  // This is a placeholder function. The watchdog needs a callback
  // but we're petting it manually in our update method
}

void SystemMonitor::initializeWatchdog() {
  // Enable the watchdog timer with stack overflow protection
  _watchdog = new ApplicationWatchdog(WATCHDOG_TIMEOUT, SystemMonitor::watchdogHandler, 1536);
  Serial.printlnf("Watchdog timer enabled with %lu ms timeout", WATCHDOG_TIMEOUT);
}

void SystemMonitor::checkBootSequence(unsigned long currentTime) {
  // Check if we're connected to the Particle cloud
  if (Particle.connected()) {
    // Connected, wait for the specified boot delay
    if (currentTime - _bootStartTime >= BOOT_DELAY) {
      // Boot sequence complete
      _bootSequenceComplete = true;
      Serial.println("Boot sequence complete, beginning normal operation");
    }
  } else {
    // Not connected yet, check timeout
    if (currentTime - _bootStartTime >= CONNECTION_WAIT) {
      // Force continue even without connection
      _bootSequenceComplete = true;
      Serial.println("Connection wait timeout, continuing with operation");
    }
  }
} 