#include <FastLED.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define DATA_PIN    6 //data pin for LCD matrix
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    256
#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16
#define BRIGHTNESS  64

CRGB leds[NUM_LEDS];
LiquidCrystal_I2C lcd(0x27,16,2);

volatile int count = 0;
const int MAX_OCCUPANCY = 64;
const int threshold = 128; // Assuming a 0-1023 analog read range, 512 is approximately 2.5V.

// Previous readings to detect a change.
int lastReadingA2 = 0;
int lastReadingA3 = 0;

volatile unsigned long lastInterruptTime = 0;
volatile bool sensor2Active = false;
volatile bool sensor1Active = false;
class LEDMatrix {
public:
    LEDMatrix() {
        FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
        FastLED.setBrightness(BRIGHTNESS);
        memset(ledState, 0, sizeof(ledState));
    }

    void displayOccupancy(int count) {
        while (numDrops < count) {
            addDrop();
        }
        while (numDrops > count) {
            removeDrop();
        }
        FastLED.show();
    }
    

private:
    bool ledState[MATRIX_WIDTH][MATRIX_HEIGHT];
    int numDrops = 0;

    void addDrop() {
        int col = random(MATRIX_WIDTH);
        int row = findFirstEmptyRow(col);
        if (row != -1) {
            animateDrop(col, row, true);
            ledState[col][row] = true;
            numDrops++;
        }
    }

    void removeDrop() {
        int col = random(MATRIX_WIDTH);
        int row = findLowestOccupiedRow(col);
        if (row != -1) {
            animateDrop(col, row, false);
            ledState[col][row] = false;
            numDrops--;
        }
    }

    int findFirstEmptyRow(int col) {
        for (int row = MATRIX_HEIGHT - 1; row >= 0; --row) {
            if (!ledState[col][row]) {
                return row;
            }
        }
        return -1;
    }

    int findLowestOccupiedRow(int col) {
        for (int row = 0; row < MATRIX_HEIGHT; ++row) {
            if (ledState[col][row]) {
                return row;
            }
        }
        return -1;
    }

    void animateDrop(int col, int endRow, bool adding) {
        int step = adding ? 1 : -1;
        for (int row = (adding ? 0 : endRow); adding ? (row <= endRow) : (row >= 0); row += step) {
            int index = xyToIndex(col, row);
            leds[index] = adding ? calculateColor(row) : CRGB::Black;
            if (!adding || row != endRow) FastLED.show();
            if (adding && row != endRow) leds[index] = CRGB::Black;
        }
    }

    CRGB calculateColor(int y) {
        return CRGB(255 - y * 255 / MATRIX_HEIGHT, y * 255 / MATRIX_HEIGHT, 0);
    }

    // int xyToIndex(int x, int y) {
    //     return y % 2 == 0 ? y * MATRIX_WIDTH + x : (y + 1) * MATRIX_WIDTH - x - 1;
    // }
    int xyToIndex(int x, int y) {
    // Rotate 90 degrees clockwise
    int newX = y;
    int newY = MATRIX_WIDTH - x - 1;

    return newY % 2 == 0 ? newY * MATRIX_HEIGHT + newX : (newY + 1) * MATRIX_HEIGHT - newX - 1;
}

};

LEDMatrix matrix;

void setup() {
  lcd.init(); 
  lcd.backlight();
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  // pinMode(A2, INPUT);
  // pinMode(A3, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(2), sensor1, RISING);
  attachInterrupt(digitalPinToInterrupt(3), sensor2, RISING);
  delay(3000);
  count = 20;
  matrix.displayOccupancy(50);
}

void loop() {
  // int currentReadingA2 = analogRead(A2);
  // int currentReadingA3 = analogRead(A3);

  // // Check if someone has passed from A2 to A3
  // if (lastReadingA2 < threshold && currentReadingA2 >= threshold && currentReadingA3 < threshold) {
  //   count++;
  //   // Delay to avoid counting the same person multiple times.
  //   delay(100);
  // }
  // // Check if someone has passed from A3 to A2
  // if (lastReadingA3 < threshold && currentReadingA3 >= threshold && currentReadingA2 < threshold) {
  //   count--;
  //   // Delay to avoid counting the same person multiple times.
  //   delay(100);
  // }

  // // Update the last readings.
  // lastReadingA2 = currentReadingA2;
  // lastReadingA3 = currentReadingA3;
  lcd.clear();
//   count += random(-25, 20);

// // Clamp the count to be within the range of 0 to MAX_OCCUPANCY
//   count = max(0, min(count, MAX_OCCUPANCY));
  lcd.backlight();

  lcd.setCursor(0, 0); // Start at character 0 on line 0
  lcd.print("Occupancy Count:");
  lcd.setCursor(0, 1); // Start at character 0 on line 1
  lcd.print(count);
// Calculate the number of LEDs to display based on the occupancy count
int d = (int)((float)count / MAX_OCCUPANCY * NUM_LEDS);
    if(d>=0 && d <= NUM_LEDS){
      matrix.displayOccupancy(d); // Simulate adding and removing drops
      delay(1000); // Pause before the next loop iteration
    }
}
// Interrupt Service Routines
void sensor1() {
  if ((millis() - lastInterruptTime) > 50) { // 50 ms debounce period
    if(sensor2Active){
      count++; // Increment count as someone has moved from sensor 2 to sensor 1
      sensor2Active = false;
    }else{
      sensor1Active = true;
    }
    lastInterruptTime = millis();
  }
}

void sensor2() {
  if ((millis() - lastInterruptTime) > 50) { // 50 ms debounce period
    if(sensor1Active){
      count--; // Decrement count as someone has moved from sensor 1 to sensor 2
      sensor1Active = false;
    }else{
      sensor2Active = true;
    }
    lastInterruptTime = millis();
  }
}

