//version 1.8
#include <LowPower.h>  // Include LowPower library
#include "RTClib.h"
#include <MINI_PCF8814.h>
#include "Thermometer.h"
#include "Clock.h"
#include "Battery.h"
#include "Beep.h"

#define INTERRUPT_PIN 2

//Create a beep object

Beep beep(LED_BUILTIN);

// Create an RTC object for DS1307
RTC_DS1307 myRtc;

// Create the display object
PCF8814 display;  // Bitbang SPI
Screen screen(display);

// Create the Thermometer object
Thermometer myThermometer(screen, /*-36.26*/-33.06, 0.1111, A0, 5);

// Create Clock object
Clock myClock(screen);

// Battery voltage reading
Battery battery;

// Global flags for interrupt
volatile bool updateFlag = false;

unsigned long lastTempUpdate = 0;
unsigned long lastHistoryUpdate = 0;

void setup() {
  Serial.begin(4800);
  battery.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  display.begin();
  display.lcd_contrast(0);  // Contrast to minimum

  Serial.println(F("STARTING..."));
  display.clear();
  display.setCursor(0, 0);
  display.print(F("STARTING..."));

  // Initialize RTC
  checkAndAdjustRTC();

  // Set up the SQW/OUT pin to output a 1Hz square wave
  myRtc.writeSqwPinMode(DS1307_SquareWave1HZ);
  Serial.println(F("RTC SET TO SQW 1HZ"));

  delay(3000);  //needed to stabalize the ADC

  // Initialize the thermometer
  myThermometer.updateTemp();
  myThermometer.resetMinMax();


  // Set up the interrupt pin
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  myClock.begin(DateTime(1970, 1, 1, 60, 0, 0));
  // Attach interrupt to falling edge
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interruptHandler, FALLING);
  display.clear();
  screen.drawBackground();
  myThermometer.render();
  beep.shortBeep();
}

void loop() {
  if (battery.getBatteryPercentage() <= 19) {
    display.clear();
    display.setCursor(0, 0);
    display.print(F("LOW BAT"));
    Serial.println(F("OFF"));
    myRtc.writeSqwPinMode(DS1307_OFF);
    screen.drawBatSimble(battery.getBatteryPercentage());
    goToPermanentSleep();
  }
  if (updateFlag) {
   
    //debug
    //uint16_t vcc = battery.getVcc();
    //Serial.print(F("Vcc in mV: "));
    //Serial.println(vcc);
    //Serial.print("DATA: ");
    //Serial.print(myClock.getDay());
    //Serial.print("/");
    //Serial.print(myClock.getMonth());
    //Serial.print("/");
    //Serial.println(myClock.getYear());
    


    DateTime now = myRtc.now();

    myClock.setDateTime(now);

    //myThermometer.updateTemp();

    screen.drawTemp(myThermometer.getTemp());

    myClock.render();
    myThermometer.render();
    screen.drawBatSimble(battery.getUsableBatteryPercentage());
    //myThermometer.updateTempHistory(now);
    if (!now.minute() && !now.second()) {  //every hour
      //myThermometer.updateTempHistory(now);
      beep.shortBeep();
    }

    if (!now.second()) {  //every minute
      myThermometer.updateTemp();
      myThermometer.updateTempHistory(now);
      //beep.alarm_3();
    }
    

    // Set updateFlag to false, so we wait again for the next interrupt
    updateFlag = false;
  }

  // Put the Arduino into the deepest sleep mode: Power-down mode
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);  // Sleep indefinitely, wake up on interrupt
}

void interruptHandler() {
  // This interrupt handler simply sets the flag to indicate the processor should update data
  updateFlag = true;
}

void goToPermanentSleep() {
  // Disable ADC to save power
  ADCSRA &= ~(1 << ADEN);

  // Disable BOD (Brown-Out Detector) during sleep for even lower power
  noInterrupts();  // Timed sequence for BOD disable
  sleep_bod_disable();

  // Go to deep sleep and never wake up unless reset
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  // The code will never reach this point
  while (1)
    ;
}

void checkAndAdjustRTC() {
  bool rtcFail = false;

  DateTime current;

  if (!myRtc.begin()) {
    Serial.println(F("RTC not found! Time and Date need to be adjusted."));
    rtcFail = true;
  } else {
    current = myRtc.now();
    Serial.print(F("Current RTC time: "));
    Serial.print(current.year());
    Serial.print('/');
    Serial.print(current.month());
    Serial.print('/');
    Serial.print(current.day());
    Serial.print(' ');
    Serial.print(current.hour());
    Serial.print(':');
    Serial.print(current.minute());
    Serial.print(':');
    Serial.println(current.second());
  }

  Serial.println(F("Adjust time? (y/n)"));
  char response = 0;
  unsigned long startMillis = millis();

  while ((millis() - startMillis) < 10000) {
    if (Serial.available()) {
      response = Serial.read();
      Serial.println(response);  // Echo

      if (response == 'n' || response == 'N') {
        while (Serial.available()) Serial.read();
        Serial.println(F("Date and Time not adjusted by the user."));
        if (rtcFail) {
          Serial.println(F("Setting time to 1970/01/01 00:00:00"));
          myRtc.adjust(DateTime(1970, 1, 1, 0, 0, 0));  // Set fallback time
        }
        return;
      } else if (response == 'y' || response == 'Y') {
        while (Serial.available()) Serial.read();
        break;
      } else {
        Serial.println(F("Invalid input. Please enter 'y' or 'n'."));
        // Optional: reset timer if input is invalid
        startMillis = millis();  // Give the user another 10s
      }
    }
  }

  // If no valid input after 10 seconds
  if (response != 'y' && response != 'Y') {
    Serial.println(F("No valid input. Continuing without adjusting time."));
    if (rtcFail) {
      Serial.println(F("Setting time to 1970/01/01 00:00:00"));
      myRtc.adjust(DateTime(1970, 1, 1, 0, 0, 0));  // Set fallback time
    }
    return;
  }

  int attempt = 0;
  while (attempt < 5) {
    Serial.println(F("Enter new time: YYYY MM DD HH MM SS"));

    while (Serial.available()) Serial.read();  // Clear serial buffer
    while (!Serial.available())
      ;  // Wait for user to start typing

    String input = Serial.readStringUntil('\n');
    input.trim();           // Remove \r or trailing spaces
    Serial.println(input);  // Echo back

    int year, month, day, hour, minute, second;
    int valuesRead = sscanf(input.c_str(), "%d %d %d %d %d %d",
                            &year, &month, &day, &hour, &minute, &second);

    bool valid = valuesRead == 6 && year >= 2000 && year <= 2099 && month >= 1 && month <= 12 && day >= 1 && day <= 31 && hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59;

    if (valid) {
      Serial.print(F("Setting RTC to: "));
      Serial.print(year);
      Serial.print('/');
      Serial.print(month);
      Serial.print('/');
      Serial.print(day);
      Serial.print(' ');
      Serial.print(hour);
      Serial.print(':');
      Serial.print(minute);
      Serial.print(':');
      Serial.println(second);

      myRtc.adjust(DateTime(year, month, day, hour, minute, second));
      Serial.println(F("RTC time updated."));
      return;
    } else {
      Serial.println(F("Invalid time format or out-of-range values. Please try again."));
      attempt++;
    }
  }

  Serial.println(F("Too many failed attempts. Aborting time adjustment."));
}
