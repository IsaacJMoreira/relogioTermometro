#ifndef SCREEN
#define SCREEN


// A custom "degrees" symbol...
static const byte DEGREES_CHAR = 1;
static const byte degrees_glyph[] = { 0x00, 0x07, 0x05, 0x07, 0x00 };

static const byte DOWN_CHAR = 2;

static const byte down_glyph[] = {
  B00100000,  // 0x00
  B01100000,  // 0x07
  B11111111,  // 0x05
  B01100000,  // 0x07
  B00100000   // 0x00
};

static const byte UP_CHAR = 3;

static const byte up_glyph[] = {
  B00000100,  // 0x00
  B00000110,  // 0x07
  B11111111,  // 0x05
  B00000110,  // 0x07
  B00000100   // 0x00
};


#include <MINI_PCF8814.h>
#include "ScreenDefines.h"
#include "bitmaps.h"
#include "types.h"


class Screen {
private:
  PCF8814* display;
  unsigned char graphBuffer[52];
  const unsigned char* simbles[11] = {
    digit_0,
    digit_1,
    digit_2,
    digit_3,
    digit_4,
    digit_5,
    digit_6,
    digit_7,
    digit_8,
    digit_9,
    digit_separator
  };

  uint8_t prevH_0 = 255;
  uint8_t prevH_1 = 255;
  uint8_t prevM_0 = 255;
  uint8_t prevM_1 = 255;

public:
  Screen(PCF8814& myDisplay)
    : display(&myDisplay) {
    for (uint8_t col = 0; col < 26; col++) {
      uint8_t pattern = (col % 2 == 0) ? B10101010 : B01010101;
      graphBuffer[col] = pattern;       // top row (0–25)
      graphBuffer[col + 26] = pattern;  // bottom row (26–51)
    }
  }

  void drawHour(uint8_t H_0, uint8_t H_1) {
    uint8_t buffer[48];
    if (prevH_0 != H_0) {
      this->display->setCursor(hourX_0, hourY_0);
      memcpy_P(buffer, this->simbles[H_0], 48);
      this->display->drawBitmap(buffer, 16, 3);
    }
    if (prevH_1 != H_1) {
      this->display->setCursor(hourX_1, hourY_1);
      memcpy_P(buffer, this->simbles[H_1], 48);
      this->display->drawBitmap(buffer, 16, 3);
    }
    this->prevH_0 = H_0;
    this->prevH_1 = H_1;
  }

  void drawMinute(uint8_t M_0, uint8_t M_1) {
    uint8_t buffer[48];
    if (prevM_0 != M_0) {
      this->display->setCursor(minuteX_0, minuteY_0);
      memcpy_P(buffer, this->simbles[M_0], 48);
      this->display->drawBitmap(buffer, 16, 3);
    }
    if (prevM_1 != M_1) {
      this->display->setCursor(minuteX_1, minuteY_1);
      memcpy_P(buffer, this->simbles[M_1], 48);
      this->display->drawBitmap(buffer, 16, 3);
    }
    this->prevM_0 = M_0;
    this->prevM_1 = M_1;
  }

  void drawSecond(const char* string) {
    this->display->setCursor(secondX, secondY);
    this->display->print(string);
  }

  void drawDay(const char* string) {
    this->display->setCursor(dayX, dayY);
    this->display->print(string);
  }

  void drawMonth(const char* string) {
    this->display->setCursor(monthX, monthY);
    this->display->print(string);
  }

  void drawYear(const char* string) {
    this->display->setCursor(yearX, yearY);
    this->display->print(string);
  }

  void drawWeek(const char* string) {
    this->display->setCursor(weekX, weekY);
    this->display->print(string);
  }

  void drawBatSimble(uint8_t percentage) {
    this->display->setCursor(batSimbleX, batSimbleY);
    unsigned char buffer[sizeof(batSimble)];
    memcpy_P(buffer, batSimble, sizeof(batSimble));
    uint8_t bars = map(percentage, 0, 100, 0, 12);
    for (uint8_t i = 14; i >= 14 - bars; i--) {
      buffer[i] = B11111111;
    }
    this->display->drawBitmap(buffer, sizeof(batSimble), 1);
  }

  void drawTemp(float temp) {
    this->display->setCursor(tempX, tempY);
    this->display->print(temp, 1);
  }

  void drawMin(float min) {
    this->display->setCursor(minTempX, minTempY);
    this->display->print(min, 1);
  }

  void drawMax(float max) {
    this->display->setCursor(maxTempX, maxTempY);
    this->display->print(max, 1);
  }

  void drawTempChart(logEntry* tempLog, float min, float max) {

    this->display->setCursor(tempChartX, tempChartY);

    //Draws the left and right borders
    this->graphBuffer[00] = B10101010;
    this->graphBuffer[26] = B10101010;
    this->graphBuffer[25] = B01010101;
    this->graphBuffer[51] = B01010101;
    for (int i = 1; i < 25; i++) {

      const uint8_t dottedEvenTop = B00000001;
      const uint8_t dottedOddTop = B00000000;
      const uint8_t dottedEvenBottom = B00000000;
      const uint8_t dottedOddBottom = B10000000;
      uint8_t dottedTop;
      uint8_t dottedBottom;

      if (i % 2) {
        dottedTop = dottedOddTop;
        dottedBottom = dottedOddBottom;
      } else {
        dottedTop = dottedEvenTop;
        dottedBottom = dottedEvenBottom;
      }

      float temp = tempLog[i - 1].tempEntry < min ? min : tempLog[i - 1].tempEntry > max ? max
                                                                                         : tempLog[i - 1].tempEntry;
      float topRange = ceil(max);
      float bottomRange = floor(min);

      uint8_t height = map(temp, bottomRange, topRange, 0, 13);
      if (height == 0) {
        this->graphBuffer[i + 26] = B11000000 ^ dottedBottom;  //bottom
        this->graphBuffer[i] = B00000001 ^ dottedTop;          //top
      } else if (height == 7) {
        this->graphBuffer[i + 26] = B11111111 ^ dottedBottom;  //bottom
        this->graphBuffer[i] = B00000001 ^ dottedTop;          //top
      } else if (height == 13) {
        this->graphBuffer[i + 26] = B11111111 ^ dottedBottom;  //bottom
        this->graphBuffer[i] = B11111111 ^ dottedTop;          //top
      } else if ((height < 13) && (height > 7)) {
        this->graphBuffer[i + 26] = B11111111 ^ dottedBottom;  //bottom
        uint8_t topHalf = (height - 7);
        this->graphBuffer[i] = (B00000001 | ~(B11111111 >> (topHalf))) ^ dottedTop;  //top
      } else if ((height < 7) && (height > 0)) {
        this->graphBuffer[i + 26] = (B10000000 | ~(B11111111 >> (height + 1))) ^ dottedBottom;  //bottom
        this->graphBuffer[i] = B00000001 ^ dottedTop;                                           //top
      }
    }
    this->display->drawBitmap(this->graphBuffer, 26, 2);
  }

  void drawBackground() {
    //this sequence will probably change, but the idea is the same

    //separators
    this->display->setCursor(separator_HM_X, separator_HM_Y);
    unsigned char buffer[12];
    memcpy_P(buffer, digit_separator, 12);
    this->display->drawBitmap(buffer, 4, 3);
    this->display->setCursor(separator_MS_X, separator_MS_Y);
    this->display->print(separator_MS_char);
    this->display->setCursor(separator_DM_X, separator_DM_Y);
    this->display->print(separator_DM_char);
    this->display->setCursor(separator_MY_X, separator_MY_Y);
    this->display->print(separator_MY_char);
    this->display->setCursor(tempTextX, tempTextY);
    this->display->print(F("Temp:"));
    this->display->createChar(DEGREES_CHAR, degrees_glyph);
    this->display->setCursor(tempGliphX, tempGliphY);
    this->display->print("\001C");
    this->display->createChar(UP_CHAR, up_glyph);
    this->display->setCursor(maxGliphX, maxGliphY);
    this->display->print("\003");
    this->display->createChar(DOWN_CHAR, down_glyph);
    this->display->setCursor(minGliphX, minGliphY);
    this->display->print("\002");
  }
};

#endif