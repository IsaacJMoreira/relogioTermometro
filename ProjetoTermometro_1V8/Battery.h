class Battery {
private:
  long vcc = 0;
  uint8_t batteryPercentage = 0;
  uint8_t usableBatteryPercentage = 0;
  unsigned long conversionStartTime = 0;
  const unsigned long TIMEOUT = 1000;  // Timeout after 1000ms (1 second)
  bool errorFlag = false;

//LUT for battery percentage
 const uint8_t batteryLUT[19] = {
  0,   // 3300 mV
  3,   // 3350
  6,   // 3400
  10,  // 3450
  14,  // 3500
  19,  // 3550
  25,  // 3600
  32,  // 3650
  40,  // 3700
  48,  // 3750
  57,  // 3800
  65,  // 3850
  73,  // 3900
  80,  // 3950
  87,  // 4000
  92,  // 4050
  96,  // 4100
  98,  // 4150
  100  // 4200+
};



  void updateVcc() {
    // Enable ADC explicitly
    ADCSRA |= (1 << ADEN);

    // Select internal 1.1V reference, measure channel 14
    ADMUX = (1 << REFS0) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1);
    delay(2);  // Let Vref stabilize
    // Start conversion
    ADCSRA |= (1 << ADSC);

    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC))
      ;

    uint16_t result = ADC;
    if (result == 0 || result > 1023) {
      this->errorFlag = true;
      this->vcc = 0;
    } else {
      this->vcc = 1126400L / result;
    }
  }

public:
  // Constructor
  Battery() {
  }

  void begin() {

    updateVcc();  // Initialize the vcc value on object creation
  }

  // Get the current Vcc value in millivolts
  long getVcc() {
    updateVcc();  // Update Vcc value whenever it's requested
    return this->vcc;
  }

  // Check if there was an error in the ADC reading
  bool hasError() {
    return errorFlag;
  }

  // Reset the error flag
  void resetError() {
    errorFlag = false;
  }

  uint8_t getBatteryPercentage() {
  updateVcc();  // Update Vcc value whenever it's requested
  long vcc = this->getVcc() + 700;

  if (vcc < 3300) return 0;
  if (vcc >= 4200) return 100;

  uint8_t index = (vcc - 3300) / 50;
  if (index >= sizeof(batteryLUT)) index = sizeof(batteryLUT) - 1;

  this->batteryPercentage =  batteryLUT[index];

  return this->batteryPercentage;
}

uint8_t getUsableBatteryPercentage() {
  updateVcc();  // Update Vcc value
  long correctedVcc = this->getVcc() + 700;  // Add diode drop

  if (correctedVcc < 3300) return 0;
  if (correctedVcc >= 4200) return 100;

  uint8_t index = (correctedVcc - 3300) / 50;
  if (index >= sizeof(batteryLUT)) index = sizeof(batteryLUT) - 1;

  uint8_t rawPercentage = batteryLUT[index];

  // Remap 14–100% to 0–100% using Arduino's map()
  if (rawPercentage <= 14) return 0;
  if (rawPercentage >= 100) return 100;

  uint8_t adjusted = map(rawPercentage, 14, 100, 0, 100);
  this->usableBatteryPercentage = adjusted;

  return this->usableBatteryPercentage;
}


};
