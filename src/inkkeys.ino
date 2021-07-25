/*
 * E-Ink adaptive macro keyboard
 * See https://there.oughta.be/a/macro-keyboard
 * 
 * This is the main code which acts as a HID keyboard while
 * allowing to be controlled through a serial interface.
 */

#include "settings.h"           //Customize your settings in settings.h!

#include "eventsequence.h"      //Structure and constants to define the sequence of events assigned to different buttons

#include <HID-Project.h>        //HID (Keyboard/Mouse etc.)
#include <Encoder.h>            //Rotary Encoder
#include <Adafruit_NeoPixel.h>  //Digital RGB LEDs
#include <SPI.h>
#include <Wire.h>             
#include <Adafruit_GFX.h>
#include <Arduino_ST7789_Fast.h> // Hardware-specific library for ST7789

// Keys 1-6
// Encoder 7
const byte nSW = 7;
const byte SW[] = {PIN_SW1, PIN_SW2, PIN_SW3, PIN_SW4, PIN_SW5, PIN_SW6, PIN_SW7 }; //Array of switches for easy iteration
bool pressed[] = {false, false, false, false, false, false, false}; //Last state of each key to track changes
uint32_t swDebounce[] = {0L, 0L, 0L, 0L, 0L, 0L, 0L};

//Rotary encoder
Encoder rotary(PIN_ROTA, PIN_ROTB);
long rotaryPosition = 0;  //Last position to keep track of changes

Arduino_ST7789 display = Arduino_ST7789(TFT_DC, TFT_RST, TFT_CS);

void initDisplay() {
  display.init(DISP_W, DISP_W);
  const int analogOutPin = 3;   
  pinMode(analogOutPin, OUTPUT);
  digitalWrite(analogOutPin, LOW);
  analogWrite(analogOutPin, 30);
  display.fillScreen(BLACK);
}

void defaultDisplay() {
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
  display.setCursor( 14, 32);
  display.print("1-6 F13-18");
  display.setTextSize(3);
  display.setTextColor(YELLOW, BLACK);
  display.setCursor( 34, 235-27);
  display.print("MOUSEWHEEL");
}

void setup() {
  //Initialize LEDs first, for some reason they initialize blue
  //  after a power cycle which is pretty annoying.
  
  Serial.begin(115200);

  initDisplay();
  initLEDs();

  defaultAssignment();
  
  //Greeting on serial
  Serial.println("Inkkeys");
  
  //Set pin modes for keys
  for (int i = 0; i < nSW; i++) {
    pinMode(SW[i], INPUT_PULLUP);
  }

  //Set rotary encoder to zero
  rotary.write(rotaryPosition);

  //Init HID.
  //These three should provide most desired functions and as the 32u4
  //on the Pro Micro provides 6 endpoints (-1 for serial), we can
  //use the single report variants for compatibility.
  BootKeyboard.begin();
  BootMouse.begin();
  SingleConsumer.begin();

  //Show LED greeting to confirm completion
  Serial.println("= Inkkeys =");
  
  ledGreeting(600);
    
  defaultKeyLed();
  defaultDisplay();

  handleLightSensor();
}

int handleLightSensor() {
  int sensorValue = analogRead(analogInPin);
  int outputValue = map(sensorValue, 0, 1023, 10, 255);
  analogWrite(analogOutPin, outputValue);
}

void loop() {
  checkKeysAndReportChanges();
  checkRotaryEncoderAndReportChanges();
  handleSerialInput();
}

//Called when state of key has changed. Checks debounce time
//and returns false if event should be filtered. Otherwise it
//returns true and resets the debounce timer.
bool debounce(byte i) {
  long t = millis();
  if (t - swDebounce[i] < DEBOUNCE_TIME)
    return false;
  else {
    swDebounce[i] = t;
    return true;
  }
}

//Check if keys have changed and report any changes
void checkKeysAndReportChanges() {
  for (int i = 0; i < nSW; i++) {
    int state = digitalRead(SW[i]);
    if (state == LOW && !pressed[i]) {
      if (debounce(i)) {
        pressed[i] = true;
        Serial.print(i+1);
        Serial.println("p");
        executeEvents(assignments[i][0]);
      }
    } else if (state == HIGH && pressed[i]) {
      if (debounce(i)) {
        pressed[i] = false;
        Serial.print(i+1);
        Serial.println("r");
        executeEvents(assignments[i][1]);
      }
    }
  }
}

//Check if rotary encoder position changed and report any changes
void checkRotaryEncoderAndReportChanges() {
  long rotaryNew = rotary.read();
  if (abs(rotaryNew - rotaryPosition) >= ROT_FACTOR) {
    int report = (rotaryNew-rotaryPosition)/ROT_FACTOR;
    Serial.print("R");
    Serial.println(report);
    rotaryPosition += report*ROT_FACTOR;
    for (int i = 0; i < report; i++)
      executeEvents(assignments[7][0]);
    for (int i = 0; i < -report; i++)
      executeEvents(assignments[7][1]);
  }
}
