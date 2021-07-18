/*
 * E-Ink adaptive macro keyboard
 * See https://there.oughta.be/a/macro-keyboard
 * 
 * This part is related to receiving and parsing incoming commands from serial
 */

//Buffer to process serial input
const int serialBufferSize = 256;
char serialBuffer[serialBufferSize];
byte serialBufferCount = 0;

//State while receiving an image
unsigned short expectingImageData = 0;
unsigned short imageDataTargetX, imageDataTargetY, imageDataTargetWidth, imageDataCurrentY;
uint16_t TextColor, TextBackground,BackgroundColor;


uint16_t str2rgb565(char* hx)
{
  long rgb = 0;

  for (int i= 0; i < 6; i++) {
    rgb = (rgb * 16) + x2b(hx[i]);
  }

  uint8_t red   = rgb >> 16;
  uint8_t green = rgb >> 8 & 0xff;
  uint8_t blue  = rgb & 0xff;

  uint16_t b = (blue >> 3) & 0x1f;
  uint16_t g = ((green >> 2) & 0x3f) << 5;
  uint16_t r = ((red >> 3) & 0x1f) << 11;

  return (uint16_t) (r | g | b);
}

byte x2b( char c) {
  if (isdigit(c)) {  // 0 - 9
    return c - '0';
  } 
  else if (isxdigit(c)) { // A-F, a-f
    return (c & 0xF) + 9;
  }
}


//Output an error message with index
void printErrorWithIndex(const char * msg, byte i) {
  Serial.print("E: ");
  Serial.print(msg);
  Serial.print(" at index ");
  Serial.print(i);
  Serial.println(":");
  Serial.println(serialBuffer);
}




//Called to process a small part of the serialBuffer to parse a delay event, returns the number of characters handled
byte processDelay(Event &event, byte i) {
  byte handled = 1;
  if (i+handled >= serialBufferCount) {
    printErrorWithIndex("No duration", i+handled);
    return 0;
  }
  unsigned short t = atoi(serialBuffer+i+handled);
  event.deviceAndType = DEVICE_DELAY;
  event.keycodeOrDelay = t;
  while (i+handled < serialBufferCount && serialBuffer[i+handled] >= '0' && serialBuffer[i+handled] <= '9')
    handled++;
  return handled;
}

//Called to process a small part of the serialBuffer to parse a consumer event, returns the number of characters handled
byte processConsumer(Event &event, byte i) {
  byte handled = 1;
  if (i+handled >= serialBufferCount) {
    printErrorWithIndex("No keycode", i+handled);
    return 0;
  }
  unsigned short code = atoi(serialBuffer+i+handled);
  while (i+handled < serialBufferCount && serialBuffer[i+handled] >= '0' && serialBuffer[i+handled] <= '9')
    handled++;
    
  if (i+handled >= serialBufferCount || serialBuffer[i+handled] == ' ') {
    event.deviceAndType = DEVICE_CONSUMER | TYPE_STROKE;
    event.keycodeOrDelay = code;
  } else {
    switch (serialBuffer[i+handled]) {
      case 'p':
        event.deviceAndType = DEVICE_CONSUMER | TYPE_PRESS;
        event.keycodeOrDelay = code;
        break;
      case 'r':
        event.deviceAndType = DEVICE_CONSUMER | TYPE_RELEASE;
        event.keycodeOrDelay = code;
        break;
      default:
        printErrorWithIndex("Type not supported", i+handled);
        return 0;
    }
    handled++;
  }
  return handled;
}

//Called to process a small part of the serialBuffer to parse a keyboard event, returns the number of characters handled
byte processKeyboard(Event &event, byte i) {
  byte handled = 1;
  if (i+handled >= serialBufferCount) {
    printErrorWithIndex("No keycode", i+handled);
    return 0;
  }
  unsigned short code = atoi(serialBuffer+i+handled);
  while (i+handled < serialBufferCount && serialBuffer[i+handled] >= '0' && serialBuffer[i+handled] <= '9')
    handled++;
    
  if (i+handled >= serialBufferCount || serialBuffer[i+handled] == ' ') {
    event.deviceAndType = DEVICE_KEYBOARD | TYPE_STROKE;
    event.keycodeOrDelay = code;
  } else {
    switch (serialBuffer[i+handled]) {
      case 'p':
        event.deviceAndType = DEVICE_KEYBOARD | TYPE_PRESS;
        event.keycodeOrDelay = code;
        break;
      case 'r':
        event.deviceAndType = DEVICE_KEYBOARD | TYPE_RELEASE;
        event.keycodeOrDelay = code;
        break;
      default:
        printErrorWithIndex("Type not supported", i+handled);
        return 0;
    }
    handled++;
  }
  return handled;
}

//Called to process a small part of the serialBuffer to parse a mouse event, returns the number of characters handled
byte processMouse(Event &event, byte i) {
  byte handled = 1;
  if (i+handled >= serialBufferCount) {
    printErrorWithIndex("No keycode", i+handled);
    return 0;
  }
  unsigned short code;
  switch (serialBuffer[i+handled]) {
    case 'x':
      code = MOUSEAXIS_X << 8;
      handled++;
      break;
    case 'y':
      code = MOUSEAXIS_Y << 8;
      handled++;
      break;
    case 'w':
      code = MOUSEAXIS_WHEEL << 8;
      handled++;
      break;
    default:
      byte buttoncode = atoi(serialBuffer+i+handled);
      code = (MOUSEAXIS_BUTTON << 8) | buttoncode;
      while (i+handled < serialBufferCount && serialBuffer[i+handled] >= '0' && serialBuffer[i+handled] <= '9')
        handled++;
      break;
  }

  if (i+handled >= serialBufferCount || serialBuffer[i+handled] == ' ') {
    event.deviceAndType = DEVICE_MOUSE | TYPE_STROKE;
    event.keycodeOrDelay = code;
  } else {
    switch (serialBuffer[i+handled]) {
      case 'p':
        event.deviceAndType = DEVICE_MOUSE | TYPE_PRESS;
        event.keycodeOrDelay = code;
        handled++;
        break;
      case 'r':
        event.deviceAndType = DEVICE_MOUSE | TYPE_RELEASE;
        event.keycodeOrDelay = code;
        handled++;
        break;
      case 'i':
        handled++;
        if (i+handled >= serialBufferCount) {
          printErrorWithIndex("Increment without value", i+handled);
          return 0;
        }
        int8_t inc = atoi(serialBuffer+i+handled);
        event.deviceAndType = DEVICE_MOUSE | TYPE_INCREMENT;
        event.keycodeOrDelay = code | (inc & 0xff);
        while (i+handled < serialBufferCount && ((serialBuffer[i+handled] >= '0' && serialBuffer[i+handled] <= '9') || serialBuffer[i+handled] <= '-'))
          handled++;
        break;
      default:
        printErrorWithIndex("Type not supported", i+handled);
        return 0;
    }
    return handled;
  }
}

void processText() {
  if (serialBufferCount < 4 || serialBuffer[1] != ' ') {
    printErrorWithIndex("Bad format", 1);
    return;
  }
  
  byte slot;
  
  if (serialBuffer[2] >= '0' && serialBuffer[2] <= '7') {
    slot = serialBuffer[2] - '0';
    byte i,offset;
    switch (slot)
    {
    case 0:
        // T 0 FF00FF FF00FF FFFFFF
        BackgroundColor = str2rgb565(&serialBuffer[18]);
        TextBackground = str2rgb565(&serialBuffer[11]);
        TextColor = str2rgb565(&serialBuffer[4]);     
        display.fillScreen(BackgroundColor); 
        display.setTextColor(TextColor,TextBackground);
      break;
    case 7:
        display.setTextSize(4);
        display.setCursor( 34, 235-27);
        offset = 4;
      break;
    default:
        display.setCursor( 14, slot*27);
        offset = 2;
      break;
    }
    if (slot > 0 ) {
      display.setTextSize(3);
      for (i=offset ; i < serialBufferCount; i++)
      {
        display.print(serialBuffer[i]);
      }
    } 
  }
  else {
    printErrorWithIndex("Bad format", 2);
    return;
  }
}

//Called if the serial input finds a command starting with "A" which assigns the buttons
void processAssignCommand() {
  if (serialBufferCount < 4 || serialBuffer[1] != ' ') {
    printErrorWithIndex("Bad format", 1);
    return;
  }
  
  byte key, pr;
  if (serialBuffer[2] >= '1' && serialBuffer[2] <= '9')
    key = serialBuffer[2] - '1';
  else if (serialBuffer[2] == 'R')
    key = 9;
  else {
    printErrorWithIndex("Bad format", 2);
    return;
  }
  switch(serialBuffer[3]) {
    case 'p':
    case '+':
      pr = 0;
      break;
    case 'r':
    case '-':
      pr = 1;
      break;
    default:
      pr = 0xff;
      break;
  }
  if (pr == 0xff) {
    printErrorWithIndex("Bad format", 3);
    return;
  }
  
  byte i = 4;
  byte event = 0;
  // bool error = false;
  while (i < serialBufferCount && event < N_EVENTS) {
    if (serialBuffer[i] != ' ') {
      printErrorWithIndex("Event not separated by white-space", i);
      return;
    }
    i++;
    if (i+1 >= serialBufferCount) {
      printErrorWithIndex("Sudden end", i);
      return;
    }

    byte processed = 0;
    switch (serialBuffer[i]) {
      case 'd':
        processed = processDelay(assignments[key][pr][event], i);
        break;
      case 'c':
        processed = processConsumer(assignments[key][pr][event], i);
        break;
      case 'k':
        processed = processKeyboard(assignments[key][pr][event], i);
        break;
      case 'm':
        processed = processMouse(assignments[key][pr][event], i);
        break;
      default:
        printErrorWithIndex("Unknown device", i);
    }
    
    if (processed == 0)
      break;
    event++;
    i += processed;
  }
  for (; event < N_EVENTS; event++) {
    assignments[key][pr][event].deviceAndType = DEVICE_NONE;
  }
}

void processKeyboardLed() {
  // K 1 ff0000
  byte key;
  
  if (serialBufferCount < 6+1 || serialBuffer[1] != ' ') {
    printErrorWithIndex("Bad format", 1);
    return;
  }

  key = serialBuffer[2] - '1';
  switch (key)
  {
    case 3: 
        key=5; 
        break;
    case 5: 
        key=3; 
        break;
    default:
      break;
  }

  keyleds.setPixelColor(key, strtol(&serialBuffer[3], NULL, 16));
  keyleds.show();
}

void processDisplayCommand() {
  if (serialBufferCount < 3 || serialBuffer[1] != ' ') {
    Serial.println("E: Bad format.");
    return;
  }
  byte i = 2;
  imageDataTargetX = atoi(serialBuffer + i);
  while (i < serialBufferCount && serialBuffer[i] >= '0' && serialBuffer[i] <= '9')
    i++;
  if (i+1 >= serialBufferCount || serialBuffer[i] != ' ') {
    Serial.println("E: Bad format.");
    return;
  }
  i++;
  imageDataTargetY = atoi(serialBuffer + i);
  while (i < serialBufferCount && serialBuffer[i] >= '0' && serialBuffer[i] <= '9')
    i++;
  if (i+1 >= serialBufferCount || serialBuffer[i] != ' ') {
    Serial.println("E: Bad format.");
    return;
  }
  i++;
  imageDataTargetWidth = atoi(serialBuffer + i);
  while (i < serialBufferCount && serialBuffer[i] >= '0' && serialBuffer[i] <= '9')
    i++;
  if (i+1 >= serialBufferCount || serialBuffer[i] != ' ') {
    Serial.println("E: Bad format.");
    return;
  }
  i++;
  unsigned short imageDataTargetHeight = atoi(serialBuffer + i);
  expectingImageData = imageDataTargetWidth*imageDataTargetHeight/8;
  imageDataCurrentY = imageDataTargetY;
}

void processInfoCommand() {
  if (serialBufferCount > 1) {
    Serial.println("E: Bad format.");
    return;
  }
  Serial.println("Inkkeys");
  Serial.println("TEST 0");
  Serial.print("N_LED ");
  Serial.println(N_LED);
  Serial.print("DISP_W ");
  Serial.println(DISP_W);
  Serial.print("DISP_H ");
  Serial.println(DISP_H);
  Serial.print("ROT_CIRCLE_STEPS ");
  Serial.println(ROT_CIRCLE_STEPS);
  Serial.println("Done");
}

void processLEDCommand() {
  if (serialBufferCount != 7*N_LED+1) {
    Serial.println("E: Bad format.");
    return;
  }
  for (byte i = 0; i < N_LED; i++) {
    leds.setPixelColor(i, strtol(&serialBuffer[7*i+1], NULL, 16));
  }
  leds.show();
}

//Read from Serial in and react to enter (carriage return)
void handleSerialInput() {
  if (Serial.available() > 0) {
    char c = Serial.read();

    if (expectingImageData == 0 && c == '\n') {
      //Carriage return. Command ends and needs to be processed
      if (serialBufferCount == serialBufferSize) {
        Serial.println("E: Command too long.");
      } else if (serialBufferCount > 0) {
        serialBuffer[serialBufferCount] = 0;
        switch (serialBuffer[0]) {
          case 'A': //Assign buttons
            processAssignCommand();
            break;
          case 'D':
            processDisplayCommand();
            break;
          case 'I': //Request device info
            processInfoCommand();
            break;
          case 'L': //Set LEDs
            processLEDCommand();
            break;
          case 'K': //Keyboard Leds
            processKeyboardLed();
            break;
          case 'T': //Keyboard Leds
            processText();
            break;
          default:  //Inknown command
            Serial.print("E: Unknown command: ");
            Serial.println(serialBuffer);
            break;
        }
        
      }
      
      //Command was handled. Reset buffer
      serialBufferCount = 0;
      
    } else {
      //Just a normal character or part of a transferred image. Store it in the buffer.
      if (serialBufferCount < serialBufferSize) {
        serialBuffer[serialBufferCount] = c;
        serialBufferCount++;

        if (expectingImageData > 0) {
          expectingImageData--;
          if (serialBufferCount * 8 >= imageDataTargetWidth) {
            //display.drawImage(imageDataTargetX, imageDataCurrentY, imageDataTargetWidth, 1, serialBuffer);
            serialBufferCount = 0;
            imageDataCurrentY++;
          }
        }
      }
    }
  }
}
