#include "SPI.h"
#include "Adafruit_WS2801.h"
#include <Time.h>
#include <typedefs.h>

int yellowPin = 12;
int greenPin  = 13;
int ledCount  = 25;

RGB currentColor = { 255, 0, 0 };
float currentOpacity = 0;
long currentByteColor;

String serialInput;

Adafruit_WS2801 strip = Adafruit_WS2801(ledCount, yellowPin, greenPin);

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show();
  setTime(1357041600);
  setCurrentByteColor();
}

void loop() {
  readSerial();
  maybeTriggerAlarm();
  maybeSleep();
  adjustOpacity();
  showCurrentColor();
  delay(100);
}

//Alarms

ALARM weekdayAlarm = { false, 6, 30 };
ALARM weekendAlarm = { false, 9, 30 };

long lastAlarmCheck    = 0;
long lastTriggerAt     = 0;
long lastColorUpdateAt = 0;
float adjustOpacityBy  = 0;

float opacityChagnePerMinute = 5500.0;

void adjustOpacity(){
  if(adjustOpacityBy == 0) return;
  currentOpacity += adjustOpacityBy;
  if(currentOpacity > 100) currentOpacity = 100;
  if(currentOpacity < 0)   currentOpacity = 0;
  if(currentOpacity == 100 || currentOpacity == 0) adjustOpacityBy = 0;
  setCurrentByteColor();
}

void triggerAlarm(){
  if(now() - lastTriggerAt < 60) return;
  lastTriggerAt = now();
  adjustOpacityBy = 30.0/opacityChagnePerMinute;
}

void maybeTriggerAlarm(){
  if(now() - lastAlarmCheck < 10) return;
  ALARM currentAlarm = (weekday() == dowSaturday || weekday() == dowSunday) ? weekendAlarm : weekdayAlarm;
  if(currentAlarm.enabled && currentAlarm.hour == hour() && currentAlarm.minute == minute()) triggerAlarm();
  lastAlarmCheck = now();
}

void maybeSleep(){
  if(now() - lastColorUpdateAt > 60*60*2) adjustOpacityBy = -5.0/opacityChagnePerMinute;
}

// Color changes

void showCurrentColor() {
  for (uint8_t i=0; i < ledCount; i++) {
    strip.setPixelColor(i, currentByteColor);
  }
  strip.show();
}

void setCurrentByteColor(){
  lastColorUpdateAt = now();
  currentByteColor = (int)(currentColor.r * currentOpacity) / 100;
  currentByteColor <<= 8;
  currentByteColor |= (int)(currentColor.g * currentOpacity) / 100;
  currentByteColor <<= 8;
  currentByteColor |= (int)(currentColor.b * currentOpacity) / 100;
}

// BT comms

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
    case 'A':
      weekdayAlarm = (ALARM){ toBool(tokens[1]), atoi(tokens[2]), atoi(tokens[3]) };
      weekendAlarm = (ALARM){ toBool(tokens[4]), atoi(tokens[5]), atoi(tokens[6]) };
      break;
    case 'S':
      char buffer[30];
      sprintf(buffer, "C-%d-%d-%d", currentColor.r, currentColor.g, currentColor.b);
      Serial.println(buffer);
      sprintf(buffer, "O-%d", (int)currentOpacity);
      Serial.println(buffer);
      sprintf(buffer, "A-%c-%d-%d-%c-%d-%d", toChar(weekdayAlarm.enabled), weekdayAlarm.hour, weekdayAlarm.minute, toChar(weekendAlarm.enabled), weekendAlarm.hour, weekendAlarm.minute);      
      Serial.println(buffer);
      break;
    case 'T':
      setTime(atol(tokens[1]));
      break;
    default:
      Serial.println('?');
  }
}

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

// Helpers

char toChar(bool check){
  return check ? 'T' : 'F';
}

bool toBool(char *c){
  return c[0] == 'T';
}
