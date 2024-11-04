#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>  // Hardware-specific library
#include <Adafruit_ST7735.h>  // Hardware-specific library
#include <RoxMux.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// OLED display pin definitions
#define TFT_CS 5
#define TFT_RST 4
#define TFT_DC 2

// Used for SPI connectivity
#define TFT_SCK 18
#define TFT_MOSI 23

// Setup the ST7735 LCD Display
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Interrupt pins
const uint8_t INTERRUPT1 = 13;
const uint8_t INTERRUPT2 = 34;
const uint8_t INTERRUPT3 = 14;
const uint8_t INTERRUPT4 = 27;
const uint8_t INTERRUPT5 = 26;
const uint8_t INTERRUPT6 = 25;
const uint8_t INTERRUPT7 = 21;
const uint8_t INTERRUPT8 = 35;

#define LABEL_SET 4  // Define which label set to use (values from 0 to 4)

uint8_t DISPLAY_RANGE0[2] = { 1, 7 };
uint8_t DISPLAY_RANGE1[2] = { 0, 7 };
uint8_t DISPLAY_RANGE2[2] = { 1, 7 };
uint8_t DISPLAY_RANGE3[2] = { 1, 6 };
uint8_t DISPLAY_RANGE4[2] = { 0, 8 };

uint8_t DISPLAY_TEXT_OFFSET0[8] = { 4, 4, 4, 3, 6, 6, 6, 4  };
uint8_t DISPLAY_TEXT_OFFSET1[8] = { 4, 6, 4, 4, 3, 6, 6, 4  };
uint8_t DISPLAY_TEXT_OFFSET2[8] = { 4, 4, 6, 3, 4, 4, 6, 4  };
uint8_t DISPLAY_TEXT_OFFSET3[8] = { 4, 6, 4, 4, 4, 5, 4, 4  };
uint8_t DISPLAY_TEXT_OFFSET4[8] = { 6, 6, 6, 6, 4, 3, 4, 4  };
uint8_t DISPLAY_OFFSET[8] = { 4, 4, 4, 4, 4, 4, 4, 4  };

// Define 5 sets of labels
const char* labelsSet0[8] = { "", "Freq", "Wave", " PW", "PWM", "Sub", "Lev", "" };
const char* labelsSet1[8] = { "Freq", "Det", "Wave", "Xmod", " EG", "Sel", "Lev", "" };
const char* labelsSet2[8] = { "", "Freq", "Res", " EG", "KFol", "KVel", "HPF", "" };
const char* labelsSet3[8] = { "", "Dep", "Freq", "Dely", "Wave", "LFO", "", "" };
const char* labelsSet4[8] = { "Att", "Dec", "Sus", "Rel", "KFol", " EG", "KVel", "VLev" };

// Select the label set based on the defined value
const char** labels;


// Input pins for multiplexer (Updated to avoid conflicts with SPI0)
int yPins[13] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
int yValues[8][13];  // Array to store multiplexer values
int lowerRange = 0;
int upperRange = 8;

volatile bool updateDisplay = false;                                                               // Flag to signal that the display should be updated
volatile bool interruptTriggered[8] = { false, false, false, false, false, false, false, false };  // Flags for interrupts

#define MUX_TOTAL 2
Rox74HC165<MUX_TOTAL> mux;

// Pins for 74HC165
#define PIN_DATA 22  // pin 9 on 74HC165 (DATA)
#define PIN_LOAD 33  // pin 1 on 74HC165 (LOAD)
#define PIN_CLK 32   // pin 2 on 74HC165 (CLK))

#define SRP_TOTAL 1
Rox74HC595<SRP_TOTAL> srp;

// pins for 74HC595
#define LED_DATA 15   // pin 14 on 74HC595 (DATA)
#define LED_CLK 16    // pin 11 on 74HC595 (CLK)
#define LED_LATCH 17  // pin 12 on 74HC595 (LATCH)
#define LED_PWM -1    // pin 13 on 74HC595

TaskHandle_t TaskDisplay;  // Handle for the display update task

// Interrupt handlers
void IRAM_ATTR xPin1() {
  interruptTriggered[0] = true;
}
void IRAM_ATTR xPin2() {
  interruptTriggered[1] = true;
}
void IRAM_ATTR xPin3() {
  interruptTriggered[2] = true;
}
void IRAM_ATTR xPin4() {
  interruptTriggered[3] = true;
}
void IRAM_ATTR xPin5() {
  interruptTriggered[4] = true;
}
void IRAM_ATTR xPin6() {
  interruptTriggered[5] = true;
}
void IRAM_ATTR xPin7() {
  interruptTriggered[6] = true;
}
void IRAM_ATTR xPin8() {
  interruptTriggered[7] = true;
}

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Initialize the multiplexer
  mux.begin(PIN_DATA, PIN_LOAD, PIN_CLK);
  srp.begin(LED_DATA, LED_LATCH, LED_CLK, LED_PWM);

  // Set up interrupt pins and attach interrupts
  pinMode(INTERRUPT1, INPUT_PULLUP);
  attachInterrupt(INTERRUPT1, xPin1, FALLING);

  pinMode(INTERRUPT2, INPUT_PULLUP);
  attachInterrupt(INTERRUPT2, xPin2, FALLING);

  pinMode(INTERRUPT3, INPUT_PULLUP);
  attachInterrupt(INTERRUPT3, xPin3, FALLING);

  pinMode(INTERRUPT4, INPUT_PULLUP);
  attachInterrupt(INTERRUPT4, xPin4, FALLING);

  pinMode(INTERRUPT5, INPUT_PULLUP);
  attachInterrupt(INTERRUPT5, xPin5, FALLING);

  pinMode(INTERRUPT6, INPUT_PULLUP);
  attachInterrupt(INTERRUPT6, xPin6, FALLING);

  pinMode(INTERRUPT7, INPUT_PULLUP);
  attachInterrupt(INTERRUPT7, xPin7, FALLING);

  pinMode(INTERRUPT8, INPUT_PULLUP);
  attachInterrupt(INTERRUPT8, xPin8, FALLING);

  // Set the label set based on LABEL_SET value
  switch (LABEL_SET) {
    case 0: labels = labelsSet0; break;
    case 1: labels = labelsSet1; break;
    case 2: labels = labelsSet2; break;
    case 3: labels = labelsSet3; break;
    case 4: labels = labelsSet4; break;
    default: labels = labelsSet0; break;  // Default to labelsSet0 if the value is out of range
  }
  switch (LABEL_SET) {
    case 0:
      lowerRange = DISPLAY_RANGE0[0];
      upperRange = DISPLAY_RANGE0[1];
      memcpy(DISPLAY_OFFSET, DISPLAY_TEXT_OFFSET0, sizeof(DISPLAY_TEXT_OFFSET0));
      break;
    case 1:
      lowerRange = DISPLAY_RANGE1[0];
      upperRange = DISPLAY_RANGE1[1];
      memcpy(DISPLAY_OFFSET, DISPLAY_TEXT_OFFSET1, sizeof(DISPLAY_TEXT_OFFSET1));
      break;
    case 2:
      lowerRange = DISPLAY_RANGE2[0];
      upperRange = DISPLAY_RANGE2[1];
      memcpy(DISPLAY_OFFSET, DISPLAY_TEXT_OFFSET2, sizeof(DISPLAY_TEXT_OFFSET2));
      break;
    case 3:
      lowerRange = DISPLAY_RANGE3[0];
      upperRange = DISPLAY_RANGE3[1];
      memcpy(DISPLAY_OFFSET, DISPLAY_TEXT_OFFSET3, sizeof(DISPLAY_TEXT_OFFSET3));
      break;
    case 4:
      lowerRange = DISPLAY_RANGE4[0];
      upperRange = DISPLAY_RANGE4[1];
      memcpy(DISPLAY_OFFSET, DISPLAY_TEXT_OFFSET4, sizeof(DISPLAY_TEXT_OFFSET4));
      break;
    default:
      lowerRange = DISPLAY_RANGE1[0];
      upperRange = DISPLAY_RANGE1[1];
      break;
  }
  // Initialize the display
  tft.init(240, 320);
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);

  // Create a task to run the display update on core 0
  xTaskCreatePinnedToCore(
    displayTask,    // Function to implement the task
    "DisplayTask",  // Name of the task
    10000,          // Stack size in words
    NULL,           // Task input parameter
    1,              // Priority of the task
    &TaskDisplay,   // Task handle
    0               // Core where the task should run
  );
}

void loop() {
  // Check if any interrupt has triggered and update accordingly
  for (int i = 0; i < 8; i++) {
    if (interruptTriggered[i]) {
      interruptTriggered[i] = false;  // Reset the flag

      delayMicroseconds(70);  // Small delay to allow signals to settle

      mux.update();  // Perform the multiplexer update
      for (uint8_t j = 0; j < 13; j++) {
        yValues[i][j] = mux.read(j);
      }
      updateDisplay = true;  // Set the flag to update the display
    }
  }
  srp.update();  // update all the LEDs in the buttons
}

// Task to update the display
void displayTask(void* parameter) {
  const int row0Margin = 10;      // Margin between row 0 and the rest of the rows
  static bool updateRow0 = true;  // Flag to determine if row 0 needs to be redrawn

  while (true) {
    if (updateDisplay) {
      updateDisplay = false;  // Reset the flag

      // Check if row 0 needs updating
      for (int x = lowerRange; x < upperRange; x++) {
        static int prevYValues[8] = { 0 };  // Store previous values for row 0
        if (yValues[x][0] != prevYValues[x]) {
          prevYValues[x] = yValues[x][0];  // Update stored value
          updateRow0 = true;               // Mark that row 0 needs to be updated
        }
      }

      // Draw row 0 if needed
      if (updateRow0) {
        updateRow0 = false;  // Reset row 0 update flag

        for (int x = lowerRange; x < upperRange; x++) {
          int xPos = x * (SCREEN_WIDTH / 8);                            // Normal x-coordinate calculation
          int yPos = SCREEN_HEIGHT - ((0 + 1) * (SCREEN_HEIGHT / 13));  // Calculate yPos for row 0

          uint16_t fillColor, textColor;

          if (yValues[x][0] == 0) {
            fillColor = ST7735_YELLOW;  // Yellow background when not selected
            textColor = ST7735_BLACK;   // Black text when not selected
            srp.writePin(x, HIGH);
          } else {
            fillColor = ST7735_BLACK;   // Black background when selected
            textColor = ST7735_YELLOW;  // Yellow text when selected
            srp.writePin(x, LOW);
          }

          // Draw the box for the label
          tft.fillRect(xPos, yPos, SCREEN_WIDTH / 8 - 10, SCREEN_HEIGHT / 13 - 1, fillColor);

          // Set text color and draw the label
          tft.setTextColor(textColor, fillColor);                    // Set both text color and background to create an inverted effect
          tft.setCursor(xPos + DISPLAY_OFFSET[x], yPos + (SCREEN_HEIGHT / 13) / 4);  // Adjust cursor to center text in box
          tft.print(labels[x]);
          tft.drawRect(xPos, yPos, SCREEN_WIDTH / 8 - 10, SCREEN_HEIGHT / 13 - 1, ST7735_YELLOW);
        }
      }

      // Draw rows 1-12 as usual
      for (int y = 1; y < 13; y++) {
        for (int x = lowerRange; x < upperRange; x++) {
          int xPos = x * (SCREEN_WIDTH / 8);                                         // Normal x-coordinate calculation
          int yPos = SCREEN_HEIGHT - ((y + 1) * (SCREEN_HEIGHT / 13)) - row0Margin;  // Inverted y-coordinate calculation with margin

          uint16_t fillColor = (yValues[x][y] == 0) ? ST7735_CYAN : ST7735_BLACK;  // Use cyan when not selected, black when selected
          tft.fillRect(xPos, yPos + 4, SCREEN_WIDTH / 8 - 10, SCREEN_HEIGHT / 13 - 1, fillColor);
        }
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);  // Small delay to prevent watchdog timer reset
  }
}