#include "FlowSensor.h"
#include "Storage.h"

// Initialize static members
volatile unsigned long FlowSensor::_customerPulseCount = 0;
volatile unsigned long FlowSensor::_technicalPulseCount = 0;
volatile bool FlowSensor::_pulseDetected = false;
FlowSensor* FlowSensor::_instance = nullptr;

FlowSensor::FlowSensor(int sensorPin, int ledPin, float pulsesPerGallon) :
  _sensorPin(sensorPin),
  _ledPin(ledPin),
  _pulsesPerGallon(pulsesPerGallon),
  _gallonsPerPulse(1.0 / pulsesPerGallon),
  _lastCustomerPulseCount(0),
  _lastTechnicalPulseCount(0),
  _flowActive(false),
  _flowStartPulse(0),
  _inactivityTimer(0),
  _lastCheckTime(0),
  _accumulatedGallons(0.0),
  _lifetimeGallons(0.0),
  _dailyGallonsTotal(0.0),
  _flowEventsToday(0),
  _hoursElapsed(0)
{
  _instance = this; // Store instance for static callbacks
}

void FlowSensor::begin() {
  pinMode(_sensorPin, INPUT_PULLUP);
  pinMode(_ledPin, OUTPUT);
  
  // Load persistent values from EEPROM
  _lifetimeGallons = Storage::loadLifetimeGallons();
  _dailyGallonsTotal = Storage::loadDailyGallons();
  _flowEventsToday = Storage::loadFlowEvents();
  _hoursElapsed = Storage::loadHoursElapsed();
  
  // Set up the interrupt
  attachInterrupt(_sensorPin, FlowSensor::pulseCounterStatic, FALLING);
  
  Serial.printlnf("Flow meter initialized with %.1f pulses per gallon", _pulsesPerGallon);
  Serial.printlnf("Lifetime gallons from storage: %.2f", _lifetimeGallons);
}

void FlowSensor::update() {
  // Handle any pending pulse indication (LED control)
  if (_pulseDetected) {
    digitalWrite(_ledPin, HIGH);
    _pulseDetected = false;
  }
  
  // Check flow status
  unsigned long currentTime = millis();
  
  // Check flow rate at regular intervals
  if (currentTime - _lastCheckTime >= FLOW_CHECK_INTERVAL) {
    checkFlow(currentTime);
    
    // Turn off LED if no flow (LED would be turned on by interrupt if there's flow)
    if (!_flowActive) {
      digitalWrite(_ledPin, LOW);
    }
  }
}

void FlowSensor::checkFlow(unsigned long currentTime) {
  _lastCheckTime = currentTime;
  
  // Calculate pulses in the last interval
  unsigned long newCustomerPulses = _customerPulseCount - _lastCustomerPulseCount;
  _lastCustomerPulseCount = _customerPulseCount;
  _lastTechnicalPulseCount = _technicalPulseCount;
  
  // Use customer counter for flow detection
  if (newCustomerPulses > MIN_PULSE_THRESHOLD) {
    // Significant flow detected
    handleFlowStart(newCustomerPulses);
    _inactivityTimer = currentTime;
  } else {
    // Check if flow has stopped after being active
    if (_flowActive && (currentTime - _inactivityTimer >= FLOW_TIMEOUT)) {
      handleFlowEnd(currentTime);
    }
  }
  
  // Debug output - can be disabled in production for power savings
  float customerGallons = _customerPulseCount * _gallonsPerPulse;
  float technicalGallons = _technicalPulseCount * _gallonsPerPulse;
  Serial.printlnf("Customer: %lu (%.2f gal), Technical: %lu (%.2f gal)", 
                 _customerPulseCount, customerGallons, 
                 _technicalPulseCount, technicalGallons);
}

void FlowSensor::handleFlowStart(unsigned long newCustomerPulses) {
  if (!_flowActive) {
    // Flow just started
    _flowActive = true;
    _flowStartPulse = _customerPulseCount - newCustomerPulses;
    _flowEventsToday++;
    Serial.println("Flow started");
  }
}

void FlowSensor::handleFlowEnd(unsigned long currentTime) {
  // Flow has been inactive for timeout period
  float gallons = (_customerPulseCount - _flowStartPulse) * _gallonsPerPulse;
  
  if (gallons > MIN_GALLONS_THRESHOLD) {
    // Add to accumulated total
    _accumulatedGallons += gallons;
    
    // Update daily total
    _dailyGallonsTotal += gallons;
    
    // Update lifetime gallons
    _lifetimeGallons += gallons;
    
    // Save to EEPROM
    Storage::saveDailyGallons(_dailyGallonsTotal);
    Storage::saveLifetimeGallons(_lifetimeGallons);
    Storage::saveFlowEvents(_flowEventsToday);
    
    Serial.printlnf("Flow ended. Added: %.2f gallons. Accumulated: %.2f gallons", 
                   gallons, _accumulatedGallons);
    Serial.printlnf("Daily total: %.2f gallons", _dailyGallonsTotal);
    Serial.printlnf("Lifetime gallons: %.2f", _lifetimeGallons);
  }
  
  // Reset for next flow event
  _flowActive = false;
  Serial.printlnf("Flow ended. Total: %.2f gallons", gallons);
}

void FlowSensor::performDailyReset() {
  _dailyGallonsTotal = 0.0;
  _flowEventsToday = 0;
  _hoursElapsed = 0;
  
  // Save to EEPROM
  Storage::saveDailyGallons(_dailyGallonsTotal);
  Storage::saveFlowEvents(_flowEventsToday);
  Storage::saveHoursElapsed(_hoursElapsed);
  
  // Reset customer pulse counter (technical counter keeps running)
  _customerPulseCount = 0;
  _lastCustomerPulseCount = 0;
  
  Serial.println("Flow sensor daily counters reset");
}

// Static pulse counter function for interrupt
void FlowSensor::pulseCounterStatic() {
  // Simply increment the counters, defer other processing to the main loop
  _customerPulseCount++;
  _technicalPulseCount++;
  _pulseDetected = true;
}

// Getters
float FlowSensor::getAccumulatedGallons() const {
  return _accumulatedGallons;
}

float FlowSensor::getDailyGallonsTotal() const {
  return _dailyGallonsTotal;
}

float FlowSensor::getLifetimeGallons() const {
  return _lifetimeGallons;
}

int FlowSensor::getFlowEventsToday() const {
  return _flowEventsToday;
}

unsigned long FlowSensor::getCustomerPulseCount() const {
  return _customerPulseCount;
}

unsigned long FlowSensor::getTechnicalPulseCount() const {
  return _technicalPulseCount;
}

bool FlowSensor::isFlowActive() const {
  return _flowActive;
}

void FlowSensor::resetAccumulatedGallons() {
  _accumulatedGallons = 0.0;
}

int FlowSensor::getHoursElapsed() const {
  return _hoursElapsed;
}

void FlowSensor::incrementHoursElapsed() {
  _hoursElapsed++;
  Storage::saveHoursElapsed(_hoursElapsed);
} 