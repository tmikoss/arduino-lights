#include "SPI.h"
#include "Adafruit_WS2801.h"

uint8_t yellowPin = 12;
uint8_t greenPin  = 13;
uint8_t ledCount  = 25;

uint32_t currentColor;

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
}

void showCurrentColor() {
  for (uint8_t i=0; i < ledCount; i++) {
    strip.setPixelColor(i, currentColor);
  }
  strip.show();
}

void receiveCommand(String command){
  int i = 0;
  char *token;
  char *tokens[20];
  char commandChars[command.length()]; 
  command.toCharArray(commandChars, command.length());
  
  token = strtok(commandChars, "-");
  while (token != NULL) {
    tokens[i++] = token;
    token = strtok(NULL, "-");
  }
  
  switch(command[0]){
    case 'C':
      currentColor = Color(atoi(tokens[1]), atoi(tokens[2]), atoi(tokens[3]));
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

uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}
