#include "SPI.h"
#include "Adafruit_WS2801.h"

typedef struct RGB {
  byte r;
  byte g;
  byte b;
};

uint8_t yellowPin = 12;
uint8_t greenPin  = 13;
uint8_t ledCount  = 25;

RGB currentColor = { 255, 255, 255 };
uint8_t currentOpacity = 10;
uint32_t currentByteColor;

String serialInput;

Adafruit_WS2801 strip = Adafruit_WS2801(ledCount, yellowPin, greenPin);

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show();
}

void loop() {
  readSerial();
  showCurrentColor(); 
  setCurrentByteColor();
}

void showCurrentColor() {
  for (uint8_t i=0; i < ledCount; i++) {
    strip.setPixelColor(i, currentByteColor);
  }
  strip.show();
}

void setCurrentByteColor(){  
  currentByteColor = (currentColor.r * currentOpacity) / 100;
  currentByteColor <<= 8;
  currentByteColor |= (currentColor.g * currentOpacity) / 100;
  currentByteColor <<= 8;
  currentByteColor |= (currentColor.b * currentOpacity) / 100;
}

void receiveCommand(String command){
  int i = 0;
  char *token;
  char *tokens[100];
  char commandChars[command.length()]; 
  command.toCharArray(commandChars, command.length());
  
  token = strtok(commandChars, "-");
  while (token != NULL) {
    tokens[i++] = token;
    token = strtok(NULL, "-");
  }
  
  switch(command[0]){
    case 'C':
      currentColor = (RGB){ atoi(tokens[1]), atoi(tokens[2]), atoi(tokens[3]) };
      setCurrentByteColor();
      break;
    case 'O':
      currentOpacity = atoi(tokens[1]);
      setCurrentByteColor();
      break;
    case 'S':
      char buffer[15];
      sprintf(buffer, "S-%d-%d-%d-%d", currentColor.r, currentColor.g, currentColor.b, currentOpacity);
      Serial.println(buffer);
      break;
    default:
      Serial.println('?');
  }
}

//Helper functions

void readSerial() {
  while (Serial.available() > 0) {
    char recieved = Serial.read();
    serialInput += recieved; 
    
    if (recieved == '\n'){
      receiveCommand(serialInput);
      serialInput = "";
    }
  }
}
