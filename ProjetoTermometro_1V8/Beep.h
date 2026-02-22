#define shortDuration 25
#define mediumDuration 50
#define longDuration 100
#define alarm_1_repetitions 3
#define alarm_2_repetitions 4
#define alarm_3_repetitions 5

class Beep {
private:
  int buzzerPin;
public:
  Beep(int pin)
    : buzzerPin(pin) {
    pinMode(this->buzzerPin, OUTPUT);
  }
  void shortBeep() {
    digitalWrite(this->buzzerPin, HIGH);
    delay(shortDuration);
    digitalWrite(this->buzzerPin, LOW);
  }
  void mediumBeep() {
    digitalWrite(this->buzzerPin, HIGH);
    delay(mediumDuration);
    digitalWrite(this->buzzerPin, LOW);
  }
  void longBeep() {
    digitalWrite(this->buzzerPin, HIGH);
    delay(longDuration);
    digitalWrite(this->buzzerPin, LOW);
  }

  void alarm_1() {
    for (uint8_t i = 0; i < alarm_1_repetitions; i++) {
      this->shortBeep();
      delay(shortDuration);
    }
  }
  void alarm_2() {
    for (uint8_t i = 0; i < alarm_2_repetitions; i++) {
      this->shortBeep();
      delay(shortDuration);
    }
  }
  void alarm_3() {
    for (uint8_t i = 0; i < alarm_3_repetitions; i++) {
      this->shortBeep();
      delay(shortDuration);
    }
  }
};