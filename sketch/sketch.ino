#include "SPI.h"
#include "Adafruit_WS2801.h"
#include <Time.h>  
#include <TimeAlarms.h>
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
  setupAlarms();
}

void loop() {
  readSerial();
  showCurrentColor(); 
  setCurrentByteColor();
  adjustOpacity();
  Alarm.delay(1);
}

//Alarms

ALARM weekdayAlarm = { false, 6, 30 };
ALARM weekendAlarm = { false, 9, 30 };

timeDayOfWeek_t weekdays[5] = {dowMonday, dowTuesday, dowWednesday, dowThursday, dowFriday};
timeDayOfWeek_t weekends[2] = {dowSaturday, dowSunday};

AlarmID_t alarms[8]; //Days are 1-indexed in time lib

float adjustOpacityBy = 0;

void adjustOpacity(){
  if(adjustOpacityBy <= 0) return;
  currentOpacity += adjustOpacityBy;
  if(currentOpacity > 100) currentOpacity = 100;
  if(currentOpacity >= 100) adjustOpacityBy = 0;
  setCurrentByteColor();
}

void triggerAlarm(){
  float minutesToMaxOpacity = 30.0;
  adjustOpacityBy = minutesToMaxOpacity/550000.0;
}

void setupAlarm(timeDayOfWeek_t day, ALARM alarm){
  if(alarms[day]) Alarm.free(alarms[day]);
  alarms[day] = Alarm.alarmRepeat(day, alarm.hour, alarm.minute, 0, triggerAlarm);
  if(!alarm.enabled) Alarm.disable(alarms[day]);
}

void setupAlarms(){
  for(int i = 0; i < 5; i++){
    setupAlarm(weekdays[i], weekdayAlarm);
  }
  for(int i = 0; i < 2; i++){
    setupAlarm(weekends[i], weekendAlarm);
  }
}

// Color changes

void showCurrentColor() {
  for (uint8_t i=0; i < ledCount; i++) {
    strip.setPixelColor(i, currentByteColor);
  }
  strip.show();
}

void setCurrentByteColor(){  
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
      setupAlarms();
      break;
    case 'S':
      char buffer[30];
      sprintf(buffer, "C-%d-%d-%d", currentColor.r, currentColor.g, currentColor.b);
      Serial.println(buffer);
      sprintf(buffer, "O-%d", currentOpacity);
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
