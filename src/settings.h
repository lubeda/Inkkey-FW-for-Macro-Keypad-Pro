//Display
// const byte PIN_DIN = 16;
// const byte PIN_CLK = 15;
// const byte PIN_CS = 19;
// const byte PIN_DC = 18;
// const byte PIN_RST = 14;
// const byte PIN_BUSY = 10;

//define display pins
#define TFT_CS    A4
#define TFT_DC     2
#define TFT_RST   A3

//Display size
const short DISP_W = 240; //Display width
const short DISP_H = 240; //Display height

// LDR and DisplayBrightness
const int analogInPin = A5;                   // Analog input pin that the LDR is attached to
const int analogOutPin = 3;   

//LEDs
const byte PIN_LED = A1; // A1
const byte N_LED = 15; //Number of LEDs

//Rotary encoder
const byte PIN_ROTA = 11;
const byte PIN_ROTB = 10;
const byte PIN_SW7 = 18;

const byte ROT_FACTOR = 4;         //Smallest reported step, typically one "click" on the encoder 
const byte ROT_CIRCLE_STEPS = 20;  //Rotary steps in a full circle

//Keys
const byte PIN_SW1 = 4;
const byte PIN_SW2 = 5;
const byte PIN_SW3 = 6;
const byte PIN_SW4 = 7;
const byte PIN_SW5 = 8;
const byte PIN_SW6 = 13;
//const byte PIN_SW8 = 8;
//const byte PIN_SW9 = 9;

const int DEBOUNCE_TIME = 50; //Debounce reject interval in milliseconds
