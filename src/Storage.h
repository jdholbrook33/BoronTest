#pragma once

#include "Particle.h"

// EEPROM address definitions
#define ADDR_MAGIC_NUMBER      0   // 4 bytes
#define ADDR_VERSION           4   // 4 bytes
#define ADDR_LIFETIME_GALLONS  8   // 4 bytes
#define ADDR_DAILY_GALLONS     12  // 4 bytes
#define ADDR_FLOW_EVENTS       16  // 4 bytes
#define ADDR_HOURS_ELAPSED     20  // 4 bytes
#define ADDR_WATCHDOG_RESETS   24  // 4 bytes
#define ADDR_DAILY_RESET_TIME  28  // 4 bytes

// Magic number to check if EEPROM is initialized
#define STORAGE_MAGIC_NUMBER   0xA753B912
#define STORAGE_VERSION        1

class Storage {
public:
  // Initialize storage
  static void begin();
  
  // Load values from EEPROM
  static float loadLifetimeGallons();
  static float loadDailyGallons();
  static int loadFlowEvents();
  static int loadHoursElapsed();
  static int loadWatchdogResetCount();
  static unsigned long loadDailyResetTime();
  
  // Save values to EEPROM
  static void saveLifetimeGallons(float value);
  static void saveDailyGallons(float value);
  static void saveFlowEvents(int value);
  static void saveHoursElapsed(int value);
  static void saveWatchdogResetCount(int value);
  static void saveDailyResetTime(unsigned long value);
  
private:
  // Check if EEPROM is initialized, initialize if not
  static bool checkInitialized();
  static void initializeEEPROM();
  
  // Template functions for EEPROM operations
  template<typename T>
  static T readValue(int address);
  
  template<typename T>
  static void writeValue(int address, T value);
}; 