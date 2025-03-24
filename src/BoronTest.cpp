#include "Particle.h"
#include "FlowSensor.h"
#include "DataReporter.h"
#include "SystemMonitor.h"
#include "Storage.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

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

// Daily reset tracking
unsigned long dailyResetTime;
const unsigned long DAILY_RESET_INTERVAL = 86400000; // 24 hours in milliseconds

// Print current time
void printCurrentTime() {
  if (Time.isValid()) {
    Serial.printlnf("Current time: %s", Time.timeStr().c_str());
    Serial.printlnf("Unix timestamp: %lu", Time.now());
    Serial.printlnf("Time is synced: %s", Time.isValid() ? "YES" : "NO");
  } else {
    Serial.println("Time is not yet synchronized");
  }
}

void setup() {
  Serial.begin(9600); 
  delay(1000); // Brief delay for serial stability
  
  Serial.println("Pool Flow Monitor Initializing");
  
  // Initialize Storage system
  Storage::begin();
  
  // Initialize daily reset time from storage
  dailyResetTime = Storage::loadDailyResetTime();
  
  // Initialize daily reset time if it's zero (first boot)
  if (dailyResetTime == 0) {
    dailyResetTime = millis();
    Storage::saveDailyResetTime(dailyResetTime);
    Serial.println("Initializing daily counter");
  }
  
  // Wait for time synchronization
  Particle.syncTime();
  waitFor(Time.isValid, 30000); // Wait up to 30 seconds for time sync
  printCurrentTime();
  
  // Initialize all system components
  systemMonitor.begin();
  flowSensor.begin();
  dataReporter.begin();
  
  Serial.println("Initialization complete, waiting for boot sequence...");
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
    
    // Periodically check and print time
    static unsigned long lastTimeCheck = 0;
    if (currentTime - lastTimeCheck >= 60000) { // Every minute
      printCurrentTime();
      lastTimeCheck = currentTime;
    }
    
    // Check if it's time for daily reset (24 hours)
    if (currentTime - dailyResetTime >= DAILY_RESET_INTERVAL) {
      Serial.println("Performing daily reset");
      
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