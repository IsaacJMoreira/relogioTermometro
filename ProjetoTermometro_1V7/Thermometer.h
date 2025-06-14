#include "RTClib.h"
#include "Screen.h"
#include "types.h"

#define TEMP_HISTORY_SIZE 24



class Thermometer {
private:
  Screen *myScreen;
  float prevTemp = 0;
  float temp = 0;
  float prevMin = 100;
  float minTemp = 100;
  float prevMax = -100;
  float maxTemp = -100;
  float tempOffset;
  float tempCoefficient;
  int analogSource;

  logEntry tempHistory[TEMP_HISTORY_SIZE];
  uint8_t tempHistoryPointer = 0;

  void incrementTempHistoryPointer() {
    this->tempHistoryPointer = (this->tempHistoryPointer + 1) >= TEMP_HISTORY_SIZE ? 0 : this->tempHistoryPointer + 1;
  }



public:
  explicit Thermometer(Screen &screen, float tempOffset, float tempCoefficient, int analogSource)
    : myScreen(&screen),
      tempOffset(tempOffset),
      tempCoefficient(tempCoefficient),
      analogSource(analogSource) {
  }

  void updateTemp() {
    this->prevTemp = this->temp;
    long temp = 0;
    for (uint16_t i = 0; i < 1000; i++) {
      temp += analogRead(this->analogSource);  // 0 - 1023
    }
    this->temp = (temp / 1000) * this->tempCoefficient + this->tempOffset;
    saveMinMax();  //also finds and saves min and max temp
  }

  float getTemp() {
    return this->temp;
  }

  void saveMinMax() {
    this->prevMin = this->minTemp;
    this->prevMax = this->maxTemp;
    if (this->temp > this->maxTemp) {
      this->maxTemp = this->temp;
    }
    if (this->temp < this->minTemp) {
      this->minTemp = this->temp;
    }
  }

  float getMin() {
    return this->minTemp;
  }

  float getMax() {
    return this->maxTemp;
  }

  void resetMinMax() {  //getMinMax to be the same value as temp -> allows new min max tracking.
    this->minTemp = this->temp;
    this->maxTemp = this->temp;
  }

  float getTempCoefficient() {
    return this->tempCoefficient;
  }

  float getTempOffset() {
    return this->tempOffset;
  }

  void setTempCoefficient(float newCoefficient) {
    this->tempCoefficient = newCoefficient;
  }

  void setTempOffset(float newOffset) {
    this->tempOffset = newOffset;
  }

  void updateTempHistory(DateTime dateTime) {
    this->tempHistory[this->tempHistoryPointer].tempEntry = this->getTemp();
    this->tempHistory[this->tempHistoryPointer].dateTimeEntry = dateTime;
    incrementTempHistoryPointer();
    this->myScreen->drawTempChart(this->tempHistory, this->minTemp, this->maxTemp);
  }

  logEntry *getTempHistory() {
    return this->tempHistory;
  }

  logEntry getLogEntry(uint8_t pointer) {
    if (pointer >= TEMP_HISTORY_SIZE) {
      return logEntry{ DateTime(1970, 1, 1, 0, 0, 0), 0.0 };
    }
    return this->tempHistory[pointer];
  }

  void render() {
    if (this->prevTemp != this->temp) {
      this->myScreen->drawTemp(this->temp);
    }
    if ((this->maxTemp != this->prevMax) || this->minTemp != this->prevMin) {
      this->myScreen->drawTempChart(this->tempHistory, this->minTemp, this->maxTemp);      
      if(this->minTemp != this->prevMin) this->myScreen->drawMin(this->minTemp);
      if(this->maxTemp != this->prevMax) this->myScreen->drawMax(this->maxTemp);
    }
  }
};
