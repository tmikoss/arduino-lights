#include "SPI.h"
#include "Adafruit_WS2801.h"

uint8_t yellowPin = 11;
uint8_t greenPin  = 13;
uint8_t ledCount  = 25;

uint32_t currentColor = 0;

Adafruit_WS2801 strip = Adafruit_WS2801(ledCount, yellowPin, greenPin);

void setup() {
  strip.begin();
  strip.show();
 
  currentColor = Color(0, 255, 0);
}

void loop() {
  showCurrentColor();
}

void showCurrentColor() {
  for (uint8_t i=0; i < ledCount; i++) {
    strip.setPixelColor(i, currentColor);
  }
  strip.show();
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
