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

const unsigned long sensorTimeout = 500; //etc
const int MAX_OCCUPANCY = 64;

volatile int count = 0;
volatile unsigned long lastInterruptTime = 0;
volatile bool sensor2Active = false;
volatile bool sensor1Active = false;
volatile unsigned long lastSensor1ActiveTime = 0;
volatile unsigned long lastSensor2ActiveTime = 0;
volatile bool change = true;

class LEDMatrix {
public:
    LEDMatrix() {
        FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
        FastLED.setBrightness(BRIGHTNESS);
        memset(ledState, 0, sizeof(ledState));
        sequencePosition = -1; // Initialize sequencePosition to -1
        initializeRoundRobinSequence();
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

    
    int roundRobinSequence[MATRIX_WIDTH];
    int sequencePosition = -1;

    void initializeRoundRobinSequence() {
        for (int i = 0; i < MATRIX_WIDTH; ++i) {
            roundRobinSequence[i] = i;
        }
        shuffleSequence();
    }

    void shuffleSequence() {
        for (int i = MATRIX_WIDTH - 1; i > 0; --i) {
            int j = random(i + 1);
            int temp = roundRobinSequence[i];
            roundRobinSequence[i] = roundRobinSequence[j];
            roundRobinSequence[j] = temp;
        }
    }

    int getNextColumnForAdd() {
          sequencePosition++;
      if (sequencePosition >= MATRIX_WIDTH) {
          shuffleSequence();
          sequencePosition = 0;
      }
      return roundRobinSequence[sequencePosition];
    }

    int getNextColumnForRemove() {
          if (sequencePosition < 0) {
          shuffleSequence();
          sequencePosition = MATRIX_WIDTH-1;
      }
      return roundRobinSequence[sequencePosition--];
    }
    

    void addDrop() {
        int col = getNextColumnForAdd();
        int row = findFirstEmptyRow(col);
        if (row != -1) {
            animateDrop(col, row, true);
            ledState[col][row] = true;
            numDrops++;
        }
    }

    void removeDrop() {
        int col = getNextColumnForRemove();
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
    int step = 1; // Always moving downward
    for (int row = (adding ? 0 : endRow); adding ? (row <= endRow) : (row < MATRIX_HEIGHT); row += step) {
        int index = xyToIndex(col, row);
        if (adding) {
            leds[index] = calculateColor(row); // Set the color of the LED
            FastLED.show(); // Update the LED strip
            if (row != endRow) {
                leds[index] = CRGB::Black; // Reset the animated LED if adding and not at endRow
            }
        } else {
            leds[index] = CRGB::Black; // Turn the current LED off (black spot) for removing
            FastLED.show(); // Update the LED strip
            
            if (row > endRow +1) {
                int prevIndex = xyToIndex(col, row - 1);
                leds[prevIndex] = calculateColor(row - 1); // Reset the previous LED to its original color
            }
        }
    }
    if(endRow != MATRIX_HEIGHT -1){
        leds[xyToIndex(col, MATRIX_HEIGHT -1)] = calculateColor(MATRIX_HEIGHT -1);
        FastLED.show();
    }
  }


    CRGB calculateColor(int y) {
        return CRGB(255 - y * 255 / MATRIX_HEIGHT, y * 255 / MATRIX_HEIGHT, 0);
    }
    //orientation configuration ===
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
bool sensorTriggered() {
    // Return true if any sensor interrupt has occurred
    return sensor1Active || sensor2Active;
}
void setup() {
  lcd.init(); 
  lcd.backlight();
  lcd.clear();

  pinMode(2, INPUT);
  pinMode(3, INPUT);

  attachInterrupt(digitalPinToInterrupt(2), sensor1, RISING);
  attachInterrupt(digitalPinToInterrupt(3), sensor2, RISING);

  count = 0; //change initial count value for simulation
}

void loop() {
  unsigned long currentMillis = millis();
    if (sensor1Active && (currentMillis - lastSensor1ActiveTime > sensorTimeout)) {
        sensor1Active = false;
    }
    if (sensor2Active && (currentMillis - lastSensor2ActiveTime > sensorTimeout)) {
        sensor2Active = false;
    }
    if(change){ //update display when new value
        lcd.clear();
        lcd.setCursor(0, 0); // Start at character 0 on line 0
        lcd.print("Occupancy Count:");
        lcd.setCursor(0, 1); // Start at character 0 on line 1
        lcd.print(count);
        // Clamp the count to be within the range of 0 to MAX_OCCUPANCY
        count = max(0, min(count, MAX_OCCUPANCY));
        // Calculate the number of LEDs to display based on the occupancy count
        int d = (int)((float)count / MAX_OCCUPANCY * NUM_LEDS);
        if(d>=0 && d <= NUM_LEDS){
          matrix.displayOccupancy(d); 
        }
        change = false; //reset change flag
    }
}
// Interrupt Service Routines
void sensor1() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastInterruptTime > 25) {
        if (sensor2Active) {
            count++;
            sensor2Active = false;
            change = true;
        } else {
            sensor1Active = true;
            lastSensor1ActiveTime = currentMillis;
        }
        lastInterruptTime = currentMillis;
    }
}

void sensor2() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastInterruptTime > 25) {
        if (sensor1Active) {
            count--;
            sensor1Active = false;
            change = true;
        } else {
            sensor2Active = true;
            lastSensor2ActiveTime = currentMillis;
        }
        lastInterruptTime = currentMillis;
    }
}
