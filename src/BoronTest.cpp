#include "Particle.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_INFO);

// Pin definitions
int light = D7;
const int FLOW_SENSOR_PIN = D2;
const float PULSES_PER_GALLON = 1140.0;
const unsigned long FLOW_CHECK_INTERVAL = 1000; // Check flow rate every second
const unsigned long FLOW_TIMEOUT = 20000; // 20 seconds of no significant flow

// Variables
volatile unsigned long pulseCount = 0;
unsigned long lastCheckTime = 0;
unsigned long lastPulseCount = 0;
unsigned long inactivityTimer = 0;
bool flowActive = false;
unsigned long flowStartPulse = 0;

// Interrupt function for pulse counting
void pulseCounter() {
  pulseCount++;
  digitalWrite(light, HIGH); // Flash LED on pulse
}

// Function to publish flow data
void publishFlowData(float gallons) {
  CellularSignal sig = Cellular.RSSI();
  int signalStrength = sig.getStrength();
  
  char jsonBuffer[256];
  snprintf(jsonBuffer, sizeof(jsonBuffer), 
           "{\"device_id\":\"pool_1\",\"timestamp\":%lu,\"gallons_used\":%.2f,\"signal_strength\":%d}",
           Time.now(), gallons, signalStrength);
  
  Serial.printlnf("Publishing flow data: %s", jsonBuffer);
  Particle.publish("flow_data", jsonBuffer, PRIVATE);
  
  // Reset pulse counter after publishing
  pulseCount = 0;
  lastPulseCount = 0;
}

void setup() {
  Serial.begin(9600);
  while(!Serial) {
    delay(100);
  }

  pinMode(light, OUTPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(FLOW_SENSOR_PIN, pulseCounter, FALLING);
  
  Serial.printlnf("Flow meter initialized with %.1f pulses per gallon", PULSES_PER_GALLON);
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check flow rate periodically
  if (currentTime - lastCheckTime >= FLOW_CHECK_INTERVAL) {
    lastCheckTime = currentTime;
    
    // Calculate pulses in the last second
    unsigned long newPulses = pulseCount - lastPulseCount;
    lastPulseCount = pulseCount;
    
    if (newPulses > 5) { // Significant flow detected (adjust threshold as needed)
      // Flow is active
      if (!flowActive) {
        // Flow just started
        flowActive = true;
        flowStartPulse = pulseCount - newPulses;
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
        float gallons = (pulseCount - flowStartPulse) / PULSES_PER_GALLON;
        
        if (gallons > 0.1) {
          publishFlowData(gallons);
        }
        
        // Reset for next flow event
        flowActive = false;
        Serial.printlnf("Flow ended. Total: %.2f gallons", gallons);
      }
    }
    
    // Display current volume
    float currentGallons = pulseCount / PULSES_PER_GALLON;
    Serial.printlnf("Pulse count: %lu, Current volume: %.2f gallons", pulseCount, currentGallons);
  }
}