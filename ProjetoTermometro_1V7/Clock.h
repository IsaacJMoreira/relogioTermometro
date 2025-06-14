#include "RTClib.h"
#include "Screen.h"
// Define the Clock class, now accepting an RTC object
class Clock {
private:
  Screen* myScreen;

  char second[3] = "00";
  char day[3] = "00";
  char week[4] = "DOM";
  char month[4] = "JAN";
  char year[5] = "1970";

  const char* dayOfTheWeek[7] = { "DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB" };
  const char* monthOfTheYear[12] = { "JAN", "FEV", "MAR", "ABR", "MAI", "JUN", "JUL", "AGO", "SET", "OUT", "NOV", "DEZ" };
  DateTime dateTime;  // DateTime object to store the current time
  DateTime prevDT;


  void add0toString(uint8_t raw, char* buffer) {
    buffer[0] = '0' + raw / 10;
    buffer[1] = '0' + raw % 10;
    buffer[2] = '\0';
  }

  void yearToString(uint16_t raw, char* buffer) {
    uint16_t thousands = raw / 1000;
    raw %= 1000;
    uint16_t hundreds = raw / 100;
    raw %= 100;
    uint16_t tens = raw / 10;
    uint16_t ones = raw % 10;

    buffer[0] = '0' + thousands;
    buffer[1] = '0' + hundreds;
    buffer[2] = '0' + tens;
    buffer[3] = '0' + ones;
    buffer[4] = '\0';  // Proper null terminator
  }


public:
  // Constructor accepts an Adafruit_SSD1306 reference
  explicit Clock(Screen& screen)
    : myScreen(&screen){};

  void begin(DateTime startDT) {
    this->dateTime = startDT;

    uint8_t second = this->dateTime.second();
    uint8_t minute = this->dateTime.minute();
    uint8_t hour = this->dateTime.hour();
    uint8_t day = this->dateTime.day();
    uint8_t month = this->dateTime.month();
    int16_t year = this->dateTime.year();

    second = second + ((second == 0) ? +1 : -1);
    minute = minute + ((minute == 0) ? +1 : -1);
    hour = hour + ((hour == 0) ? +1 : -1);
    day = day + ((day == 1) ? +1 : -1);
    month = month + ((month == 1) ? +1 : -1);
    year = year + ((year == 0) ? +1 : -1);

    // Now construct a new DateTime (RTClib-style)
    this->prevDT = DateTime(year, month, day, hour, minute, second);
  }

  // Method to set the dateTime
  void setDateTime(DateTime newDT) {
    this->dateTime = newDT;
  }

  const char* getSecond() {
    return this->second;
  }

  const char* getDay() {
    return this->day;
  }
  const char* getDayOfTheWeek() {
    return this->week;
  }
  const char* getMonth() {
    return this->month;
  }

  const char* getYear() {
    return this->year;
  }

  void render() {
    if (this->prevDT.minute() != this->dateTime.minute()) {
      uint8_t minute = this->dateTime.minute();
      uint8_t M_0 = minute / 10;
      uint8_t M_1 = minute % 10;
      this->myScreen->drawMinute(M_0, M_1);
    }
    if (this->prevDT.hour() != this->dateTime.hour()) {
      uint8_t hour = this->dateTime.hour();
      uint8_t H_0 = hour / 10;
      uint8_t H_1 = hour % 10;
      this->myScreen->drawHour(H_0, H_1);
    }
    if (this->prevDT.day() != this->dateTime.day()) {
      add0toString(this->dateTime.day(), this->day);
      this->myScreen->drawDay(this->day);
    }
    if (this->prevDT.year() != this->dateTime.year()) {
      yearToString(this->dateTime.year(), this->year);
      this->myScreen->drawYear(this->year);
    }
    if (this->prevDT.dayOfTheWeek() != this->dateTime.dayOfTheWeek()) {
      strcpy(this->week, dayOfTheWeek[this->dateTime.dayOfTheWeek()]);
      this->myScreen->drawWeek(this->week);
    }
    if (this->prevDT.month() != this->dateTime.month()) {
      strcpy(this->month, monthOfTheYear[this->dateTime.month() - 1]);
      this->myScreen->drawMonth(this->month);
    }
    add0toString(this->dateTime.second(), this->second);
    this->myScreen->drawSecond(this->second);
    this->prevDT = this->dateTime;
  }

  // Method to get the current dateTime
  DateTime getDateTime() {
    return this->dateTime;
  }
};
