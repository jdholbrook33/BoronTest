#include "Particle.h"
#include "FlowSensor.h"
#include "DataReporter.h"
#include "SystemMonitor.h"
#include "Storage.h"
#include "DisplayComm.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

// Reduce logging to INFO level only
SerialLogHandler logHandler(LOG_LEVEL_INFO);

// Enable reset info to track watchdog resets
STARTUP(System.enableFeature(FEATURE_RESET_INFO));

// Pin definitions
const int FLOW_SENSOR_PIN = D2;
const int LED_PIN = D7;

// Calibration value
const float PULSES_PER_GALLON = 1700.0;

// Component instances
FlowSensor flowSensor(FLOW_SENSOR_PIN, LED_PIN, PULSES_PER_GALLON);
DataReporter dataReporter(&flowSensor, "pool_1");
SystemMonitor systemMonitor;
DisplayComm displayComm(&flowSensor);

// Daily reset tracking
unsigned long dailyResetTime;
const unsigned long DAILY_RESET_INTERVAL = 86400000; // 24 hours in milliseconds

void setup() {
  delay(1000); // Brief delay for stability
  
  Log.info("Pool Flow Monitor Starting");
  
  // Initialize Storage system
  Storage::begin();
  
  // Initialize daily reset time from storage
  dailyResetTime = Storage::loadDailyResetTime();
  
  // Initialize daily reset time if it's zero (first boot)
  if (dailyResetTime == 0) {
    dailyResetTime = millis();
    Storage::saveDailyResetTime(dailyResetTime);
  }
  
  // Wait for time synchronization
  Particle.syncTime();
  waitFor(Time.isValid, 30000); // Wait up to 30 seconds for time sync
  
  // Initialize all system components
  systemMonitor.begin();
  flowSensor.begin();
  dataReporter.begin();
  displayComm.begin();
  
  Log.info("System initialized");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Update system monitor (handles watchdog and boot sequence)
  systemMonitor.update();
  
  // Only process flow and data if boot sequence is complete
  if (systemMonitor.isBootComplete()) {
    // Update flow sensor (processes pulse counts and flow detection)
    flowSensor.update();
    
    // Update data reporter (handles publishing based on intervals)
    dataReporter.update(currentTime);
    
    // Update display with current data
    displayComm.update(currentTime);
    
    // Time sync check once per hour (aligned with data publishing)
    static unsigned long lastTimeSync = 0;
    if (currentTime - lastTimeSync >= 3600000) { // 1 hour in milliseconds
      Particle.syncTime();
      lastTimeSync = currentTime;
    }
    
    // Check if it's time for daily reset (24 hours)
    if (currentTime - dailyResetTime >= DAILY_RESET_INTERVAL) {
      Log.info("Daily reset - Total gallons: %.2f", flowSensor.getDailyGallonsTotal());
      
      // Reset the flow sensor's daily counters
      flowSensor.performDailyReset();
      
      // Update daily reset time
      dailyResetTime = currentTime;
      Storage::saveDailyResetTime(dailyResetTime);
      
      // Publish diagnostic data after reset
      dataReporter.publishDiagnosticData();
    }
  }
}