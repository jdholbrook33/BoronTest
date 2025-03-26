#include "DisplayComm.h"
#include "FlowSensor.h"

// Constructor
DisplayComm::DisplayComm(FlowSensor* flowSensor) :
  _flowSensor(flowSensor),
  _lastDisplayUpdateTime(0)
{
}

// Initialize UART and other settings
void DisplayComm::begin() {
  // Initialize UART for communication with display
  initializeUART();
  
  Log.info("Display communication module initialized");
  
  // Test serial connection with slow character-by-character communication
  Log.info("Sending test pattern character by character");
  const char* testStr = "TEST-PATTERN-123";
  for (size_t i = 0; i < strlen(testStr); i++) {
    Serial1.write(testStr[i]);
    delay(100); // Delay 100ms between characters
  }
  Serial1.write('\n');
  delay(500);
  
  // Send an initial test pattern to verify communication - keep this simple
  Serial1.println("{\"init\":true}");
  Log.info("Sent initialization JSON to CYD");
  
  // Short delay to let display process
  delay(500);
  
  // Send initial data
  sendDisplayData();
  
  Log.info("Initial display data sent");
}

// Initialize UART
void DisplayComm::initializeUART() {
  // Initialize UART1 for communication with display - explicitly set to 115200 baud
  Serial1.begin(115200);
  
  // Allow time for UART to initialize
  delay(100);
  
  Log.info("UART1 initialized at 115200 baud for display communication");
}

// Update method to be called in main loop
void DisplayComm::update(unsigned long currentTime) {
  // Check if it's time to update the display
  if (currentTime - _lastDisplayUpdateTime >= DISPLAY_UPDATE_INTERVAL) {
    sendDisplayData();
  }
}

// Send data to display
void DisplayComm::sendDisplayData() {
  // Format the data as JSON
  String jsonData = formatDisplayData();
  
  // Option 1: Normal send (single write)
  // Serial1.println(jsonData);
  
  // Option 2: Slow character-by-character send
  for (size_t i = 0; i < jsonData.length(); i++) {
    Serial1.write(jsonData.charAt(i));
    delay(5); // Small delay between characters
  }
  Serial1.write('\n');
  
  // Debug output
  Log.info("Sent JSON to display: %s", jsonData.c_str());
  
  // Update last send time
  _lastDisplayUpdateTime = millis();
}

// Format JSON data to send to display
String DisplayComm::formatDisplayData() {
  // Calculate GPM from pulse counts over time
  float gpm = 0.0;
  
  // Check if flow is active
  if (_flowSensor->isFlowActive()) {
    // Get customer pulse count for gallons per minute calculation
    unsigned long customerPulseCount = _flowSensor->getCustomerPulseCount();
    
    // Calculate GPM based on recent flow
    // This is a simple way to calculate it; may need refinement based on how your flow sensor works
    static unsigned long lastPulseCount = 0;
    static unsigned long lastGpmCalcTime = 0;
    unsigned long currentTime = millis();
    
    // Calculate GPM every 5 seconds for a smooth reading
    if (currentTime - lastGpmCalcTime >= 5000) {
      unsigned long pulseDiff = customerPulseCount - lastPulseCount;
      float gallonsDiff = pulseDiff / 1700.0; // Using same calibration value as in main code
      float minutesFraction = (currentTime - lastGpmCalcTime) / 60000.0; // Convert ms to minutes
      
      if (minutesFraction > 0) {
        gpm = gallonsDiff / minutesFraction;
      }
      
      lastPulseCount = customerPulseCount;
      lastGpmCalcTime = currentTime;
    }
  }
  
  // Get total gallons
  float totalGallons = _flowSensor->getLifetimeGallons();
  
  // Get cellular signal strength (0-100%)
  CellularSignal sig = Cellular.RSSI();
  int signalStrength = sig.getStrength();
  
  // Get current time
  char timeStr[9]; // HH:MM:SS + null terminator
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", 
           Time.hour(), Time.minute(), Time.second());
  
  // Format JSON string - match the exact format used by the C3 simulator
  char jsonBuffer[128];
  snprintf(jsonBuffer, sizeof(jsonBuffer), 
           "{\"gpm\": %.1f, \"gallons\": %.1f, \"signal\": %d, \"time\": \"%s\"}",
           gpm, totalGallons, signalStrength, timeStr);
  
  return String(jsonBuffer);
} 