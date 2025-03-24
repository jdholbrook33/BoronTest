#include "Storage.h"

void Storage::begin() {
  if (!checkInitialized()) {
    initializeEEPROM();
  }
  
  Serial.println("Storage system initialized");
}

bool Storage::checkInitialized() {
  uint32_t magicNumber = readValue<uint32_t>(ADDR_MAGIC_NUMBER);
  uint32_t version = readValue<uint32_t>(ADDR_VERSION);
  
  return (magicNumber == STORAGE_MAGIC_NUMBER && version == STORAGE_VERSION);
}

void Storage::initializeEEPROM() {
  Serial.println("Initializing EEPROM storage...");
  
  // Write magic number and version
  writeValue<uint32_t>(ADDR_MAGIC_NUMBER, STORAGE_MAGIC_NUMBER);
  writeValue<uint32_t>(ADDR_VERSION, STORAGE_VERSION);
  
  // Initialize counters to 0
  writeValue<float>(ADDR_LIFETIME_GALLONS, 0.0f);
  writeValue<float>(ADDR_DAILY_GALLONS, 0.0f);
  writeValue<int>(ADDR_FLOW_EVENTS, 0);
  writeValue<int>(ADDR_HOURS_ELAPSED, 0);
  writeValue<int>(ADDR_WATCHDOG_RESETS, 0);
  writeValue<unsigned long>(ADDR_DAILY_RESET_TIME, 0);
  
  Serial.println("EEPROM storage initialized");
}

// Load functions
float Storage::loadLifetimeGallons() {
  return readValue<float>(ADDR_LIFETIME_GALLONS);
}

float Storage::loadDailyGallons() {
  return readValue<float>(ADDR_DAILY_GALLONS);
}

int Storage::loadFlowEvents() {
  return readValue<int>(ADDR_FLOW_EVENTS);
}

int Storage::loadHoursElapsed() {
  return readValue<int>(ADDR_HOURS_ELAPSED);
}

int Storage::loadWatchdogResetCount() {
  return readValue<int>(ADDR_WATCHDOG_RESETS);
}

unsigned long Storage::loadDailyResetTime() {
  return readValue<unsigned long>(ADDR_DAILY_RESET_TIME);
}

// Save functions
void Storage::saveLifetimeGallons(float value) {
  writeValue<float>(ADDR_LIFETIME_GALLONS, value);
}

void Storage::saveDailyGallons(float value) {
  writeValue<float>(ADDR_DAILY_GALLONS, value);
}

void Storage::saveFlowEvents(int value) {
  writeValue<int>(ADDR_FLOW_EVENTS, value);
}

void Storage::saveHoursElapsed(int value) {
  writeValue<int>(ADDR_HOURS_ELAPSED, value);
}

void Storage::saveWatchdogResetCount(int value) {
  writeValue<int>(ADDR_WATCHDOG_RESETS, value);
}

void Storage::saveDailyResetTime(unsigned long value) {
  writeValue<unsigned long>(ADDR_DAILY_RESET_TIME, value);
}

// Template implementations
template<typename T>
T Storage::readValue(int address) {
  T value;
  EEPROM.get(address, value);
  return value;
}

template<typename T>
void Storage::writeValue(int address, T value) {
  EEPROM.put(address, value);
} 