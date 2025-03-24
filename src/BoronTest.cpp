#include "Particle.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_INFO);

// Enable reset info to track watchdog resets
STARTUP(System.enableFeature(FEATURE_RESET_INFO));

// Pin definitions
int light = D7;
const int FLOW_SENSOR_PIN = D2;

// Global Variables
const float PULSES_PER_GALLON = 1700.0; // Rounded for simplicity with minimal error
const unsigned long FLOW_CHECK_INTERVAL = 5000; // Check flow rate every 5 seconds
const unsigned long FLOW_TIMEOUT = 20000; // 20 seconds of no significant flow
const unsigned long HOURLY_PUBLISH = 600000; // 10 minutes (600000) for testing
const unsigned long DIAGNOSTIC_PUBLISH_INTERVAL = 900000; // 15 minutes (900000) for testing
const unsigned long DAILY_RESET_INTERVAL = 86400000; // 24 hours in milliseconds
const unsigned long BOOT_DELAY = 30000; // 30 second boot delay before starting flow monitoring
const unsigned long CONNECTION_WAIT = 60000; // Maximum time to wait for connection (60 seconds)

// Firmware version
const String FIRMWARE_VERSION = "1.0.0";

// Accumulation variables
retained float accumulatedGallons = 0.0;    // Session gallons (since last publish)
retained float lifetimeGallons = 0.0;       // Persistent total lifetime gallons
retained float dailyGallonsTotal = 0.0;     // Total gallons in current 24-hour period
retained unsigned long dailyResetTime = 0;  // When the daily counter was last reset
retained int hoursElapsed = 0;              // Hours since daily reset
unsigned long lastPublishTime = 0;          // Track last publish time
unsigned long lastDiagnosticPublishTime = 0; // Track last diagnostic publish
bool bootSequenceComplete = false;          // Flag to track boot sequence completion
unsigned long bootStartTime = 0;            // When the boot sequence started

// Watchdog timer set up
const unsigned long WATCHDOG_TIMEOUT = 60000; // 60 seconds
retained int watchdogResetCount = 0;
String resetReasonStr = "Unknown";
ApplicationWatchdog *wd;

// Dual Pulse Counters
volatile unsigned long customerPulseCount = 0;   // Resets every 24 hours - for customer data
volatile unsigned long technicalPulseCount = 0;  // Never resets - for technical monitoring since boot
unsigned long lastCheckTime = 0;
unsigned long lastCustomerPulseCount = 0;
unsigned long lastTechnicalPulseCount = 0;
unsigned long inactivityTimer = 0;
bool flowActive = false;
unsigned long flowStartPulse = 0;

// Flow event tracking
retained int flowEventsToday = 0;  // Count of flow events in the current 24-hour period

// Forward declarations of functions
void translateResetReason();
void publishDiagnosticData();

void watchdogHandler() {
  // This is a placeholder function. The watchdog needs a callback
  // but we'll pet it manually in our loop
}

// Interrupt function for pulse counting - increments both counters
void pulseCounter() {
  // Only count pulses after boot sequence is complete
  if (bootSequenceComplete) {
    customerPulseCount++;
    technicalPulseCount++;
    digitalWrite(light, HIGH); // Flash LED on pulse
  }
}

// Function to publish flow data
void publishFlowData(float gallons, float hourlyAverage) {
  char jsonBuffer[256];
  snprintf(jsonBuffer, sizeof(jsonBuffer), 
           "{\"device_id\":\"pool_1\",\"timestamp\":%lu,\"gallons_used\":%.0f,\"hourly_average\":%.1f}",
           Time.now(), gallons, hourlyAverage);
  
  Serial.printlnf("Publishing flow data: %s", jsonBuffer);
  Particle.publish("flow_data", jsonBuffer, PRIVATE);
  
  // Update last publish time
  lastPublishTime = millis();
}

// Function to translate System.resetReason() to a readable string
void translateResetReason() {
  int reason = System.resetReason();
  
  switch (reason) {
    case RESET_REASON_PIN_RESET:
      resetReasonStr = "Pin Reset";
      break;
    case RESET_REASON_POWER_MANAGEMENT:
      resetReasonStr = "Power Management";
      break;
    case RESET_REASON_WATCHDOG:
      resetReasonStr = "Watchdog Timer";
      break;
    case RESET_REASON_UPDATE:
      resetReasonStr = "Firmware Update";
      break;
    case RESET_REASON_UPDATE_TIMEOUT:
      resetReasonStr = "Firmware Update Timeout";
      break;
    case RESET_REASON_FACTORY_RESET:
      resetReasonStr = "Factory Reset";
      break;
    case RESET_REASON_SAFE_MODE:
      resetReasonStr = "Safe Mode";
      break;
    case RESET_REASON_DFU_MODE:
      resetReasonStr = "DFU Mode";
      break;
    case RESET_REASON_PANIC:
      resetReasonStr = "System Panic";
      break;
    case RESET_REASON_USER:
      resetReasonStr = "User Requested";
      break;
    case RESET_REASON_UNKNOWN:
    default:
      resetReasonStr = "Unknown";
      break;
  }
  
  Serial.printlnf("Reset reason: %s (code: %d)", resetReasonStr.c_str(), reason);
}

// Function to publish diagnostic data
void publishDiagnosticData() {
  // Get current reset reason as string
  translateResetReason();
  
  // Get cellular signal strength
  CellularSignal sig = Cellular.RSSI();
  int signalStrength = sig.getStrength();
  
  // Technical data is calculated directly in the JSON formatting
  
  char jsonBuffer[256];
  snprintf(jsonBuffer, sizeof(jsonBuffer), 
           "{\"device_id\":\"pool_1\",\"timestamp\":%lu,\"firmware\":\"%s\",\"reset_count\":%d,\"reset_reason\":\"%s\",\"lifetime_gallons\":%.2f,\"daily_total\":%.2f,\"hours_elapsed\":%d,\"total_pulses\":%lu,\"flow_events_today\":%d,\"signal_strength\":%d}",
           Time.now(), FIRMWARE_VERSION.c_str(), watchdogResetCount, resetReasonStr.c_str(), 
           lifetimeGallons, dailyGallonsTotal, hoursElapsed, technicalPulseCount, flowEventsToday, signalStrength);
  
  Serial.printlnf("Publishing diagnostic data: %s", jsonBuffer);
  bool success = Particle.publish("diagnostic_data", jsonBuffer, PRIVATE);
  
  if (success) {
    Serial.printlnf("Diagnostic data published successfully");
  } else {
    Serial.printlnf("Failed to publish diagnostic data");
  }
  
  // Update last diagnostic publish time
  lastDiagnosticPublishTime = millis();
}

void setup() {
  Serial.begin(9600); 
  while(!Serial) {
    delay(100);
  }

  Serial.println("Starting boot sequence...");
  bootStartTime = millis();
  bootSequenceComplete = false;

  pinMode(light, OUTPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  
  // Set up the interrupt but don't count pulses yet
  attachInterrupt(FLOW_SENSOR_PIN, pulseCounter, FALLING);
  
  Serial.printlnf("Flow meter initialized with %.1f pulses per gallon", PULSES_PER_GALLON);

  // Check if last reset was due to watchdog
  if (System.resetReason() == RESET_REASON_WATCHDOG) {
    watchdogResetCount++;
    Serial.printlnf("Watchdog reset detected! Total count: %d", watchdogResetCount);
  }

  // Translate reset reason for diagnostic reporting
  translateResetReason();

  // Enable the watchdog timer
  wd = new ApplicationWatchdog(WATCHDOG_TIMEOUT, watchdogHandler, 1536);
  Serial.printlnf("Watchdog timer enabled with %lu ms timeout", WATCHDOG_TIMEOUT);
  
  // Initialize lastPublishTime to ensure we don't immediately publish
  lastPublishTime = millis();
  lastDiagnosticPublishTime = millis();
  
  // Initialize daily reset time if it's zero (first boot)
  if (dailyResetTime == 0) {
    dailyResetTime = millis();
    Serial.println("Initializing daily counter");
  }
  
  Serial.println("Waiting for network connection...");
  
  // Force time synchronization
  Serial.println("Requesting time sync from Particle Cloud...");
  Particle.syncTime();
  Serial.printlnf("Current time: %lu", Time.now());
}

void loop() {
  // Reset (pet) the watchdog timer at the beginning of each loop
  wd->checkin();

  unsigned long currentTime = millis();
  
  // Handle boot sequence - wait for connection and delay
  if (!bootSequenceComplete) {
    // Check if we're connected
    if (Particle.connected()) {
      // Connected, wait for the specified boot delay
      if (currentTime - bootStartTime >= BOOT_DELAY) {
        // Boot sequence complete
        bootSequenceComplete = true;
        Serial.println("Boot sequence complete, beginning normal operation");
        
        // Initialize counter values for first measurement
        lastCustomerPulseCount = customerPulseCount;
        lastTechnicalPulseCount = technicalPulseCount;
        lastCheckTime = currentTime;
        
        // Publish initial diagnostic data
        publishDiagnosticData();
      }
    } else {
      // Not connected yet, check timeout
      if (currentTime - bootStartTime >= CONNECTION_WAIT) {
        // Force continue even without connection
        bootSequenceComplete = true;
        Serial.println("Connection wait timeout, continuing with operation");
        
        // Initialize counter values for first measurement
        lastCustomerPulseCount = customerPulseCount;
        lastTechnicalPulseCount = technicalPulseCount;
        lastCheckTime = currentTime;
      }
    }
    
    // Skip the rest of the loop during boot sequence
    return;
  }
  
  // Normal flow monitoring operation - only runs after boot sequence is complete
  
  // Check flow rate periodically
  if (currentTime - lastCheckTime >= FLOW_CHECK_INTERVAL) {
    lastCheckTime = currentTime;
    
    // Calculate pulses in the last interval
    unsigned long newCustomerPulses = customerPulseCount - lastCustomerPulseCount;
    lastCustomerPulseCount = customerPulseCount;
    lastTechnicalPulseCount = technicalPulseCount;  // Still update technical counter for tracking
    
    // Use either counter for flow detection - they should match
    if (newCustomerPulses > 5) { // Significant flow detected (adjust threshold as needed)
      // Flow is active
      if (!flowActive) {
        // Flow just started
        flowActive = true;
        flowStartPulse = customerPulseCount - newCustomerPulses;
        flowEventsToday++;
        Serial.println("Flow started");
      }
      
      // Reset inactivity timer on active flow
      inactivityTimer = currentTime;
    } else {
      // Turn off LED if no significant flow
      digitalWrite(light, LOW);
      
      // Check if flow has stopped
      if (flowActive && (currentTime - inactivityTimer >= FLOW_TIMEOUT)) {
        // Flow has been inactive for timeout period
        float gallons = (customerPulseCount - flowStartPulse) / PULSES_PER_GALLON;
        
        if (gallons > 0.05) { // Small threshold for valid flow
          // Add to accumulated total
          accumulatedGallons += gallons;
          
          // Update daily total
          dailyGallonsTotal += gallons;
          
          // Update lifetime gallons
          lifetimeGallons += gallons;
          
          Serial.printlnf("Flow ended. Added: %.2f gallons. Accumulated: %.2f gallons", 
                         gallons, accumulatedGallons);
          Serial.printlnf("Daily total: %.2f gallons", dailyGallonsTotal);
          Serial.printlnf("Lifetime gallons: %.2f", lifetimeGallons);
        }
        
        // Reset for next flow event
        flowActive = false;
        Serial.printlnf("Flow ended. Total: %.2f gallons", gallons);
      }
    }
    
    // Display current volume from both counters for comparison
    float customerGallons = customerPulseCount / PULSES_PER_GALLON;
    float technicalGallons = technicalPulseCount / PULSES_PER_GALLON;
    Serial.printlnf("Customer pulse count: %lu (%.2f gallons), Technical pulse count: %lu (%.2f gallons)", 
                   customerPulseCount, customerGallons, technicalPulseCount, technicalGallons);
    
    // Check if it's time for hourly publish
    if (currentTime - lastPublishTime >= HOURLY_PUBLISH) {
      // Calculate hourly average
      float hourlyAverage = 0.0;
      if (hoursElapsed > 0) {
        hourlyAverage = dailyGallonsTotal / hoursElapsed;
      }
      
      // Publish data with hourly average - for customer display we use whole numbers
      int wholeGallons = (int)accumulatedGallons;
      publishFlowData((float)wholeGallons, hourlyAverage);
      
      // Log the publish event
      Serial.printlnf("Hourly publish: %d gallons this interval, %.1f gallons average per hour", 
                     wholeGallons, hourlyAverage);
      Serial.printlnf("Daily total: %.2f gallons over %d hours", dailyGallonsTotal, hoursElapsed);
      
      // Reset accumulated gallons for next interval
      accumulatedGallons = 0.0;
      
      // Increment hours elapsed (only if publishing)
      hoursElapsed++;
      
      // Update the publish time
      lastPublishTime = currentTime;
    }
    
    // Check if it's time for diagnostic publish
    if (currentTime - lastDiagnosticPublishTime >= DIAGNOSTIC_PUBLISH_INTERVAL) {
      Serial.println("Time to publish diagnostic data");
      publishDiagnosticData();
    }
    
    // Check if it's time for daily reset (24 hours)
    if (currentTime - dailyResetTime >= DAILY_RESET_INTERVAL) {
      Serial.println("Performing daily reset");
      
      // Reset daily counters
      dailyGallonsTotal = 0.0;
      hoursElapsed = 0;
      flowEventsToday = 0;
      
      // Reset customer pulse counter (but keep technical counter running)
      customerPulseCount = 0;
      lastCustomerPulseCount = 0;
      
      // Update daily reset time
      dailyResetTime = currentTime;
      
      // Publish diagnostic data after reset
      publishDiagnosticData();
    }
  }
}