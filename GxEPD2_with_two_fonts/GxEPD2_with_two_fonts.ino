// GxEPD2__with_two_fonts.ino by Henry Leach, adapted from
// GxEPD2_HelloWorld.ino by Jean-Marc Zingg
// www.henryleach.com

// Wiring for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 10, CLK -> 13, DIN -> 11

#include <GxEPD2_BW.h> // including both doesn't use more code or ram
#include <GxEPD2_3C.h> // including both doesn't use more code or ram

// Use two fonts for two different text sizes. 
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>

// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"

//2.9" screen is 296x128pixels
const char greeting[] = "Hello";
uint16_t counter = 0;
int16_t tbx, tby;
uint16_t tbw, tbh;


void setup()
{
  display.init();
  printGreeting();
  display.hibernate();
}

void loop() {
  printCounter();
}

void printGreeting()
{
  display.setRotation(1); //0 is 'portrait'
  display.setFont(&FreeSansBold24pt7b); //This almost fills the screen with "Wednesday"
  display.setTextColor(GxEPD_BLACK);

  display.getTextBounds(greeting, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center the bounding box by transposition of the origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;  // centres the text
  uint16_t y = 50; // Aligned in top half
  display.setFullWindow();
  display.firstPage();
  do // Print the upper part of the screen
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(greeting);
  }
  while (display.nextPage());
}

void printCounter(){

  char countText[15];
  snprintf(countText, sizeof(countText), "Counter: %i", counter);
  if(counter >= 100){
    counter = 0; 
  } else {
    counter++;
  }

  display.setFont(&FreeSansBold12pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.getTextBounds(countText, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x2 = ((display.width() - tbw) / 2) - tbx;
  uint16_t y2 = 100;
  //args are(x-start, y-start, x-end, y-end)
  //absolute on display. This covers the bottom half.
  display.setPartialWindow(0, display.height() / 2, display.width(), display.height());
  display.firstPage();
  do {
    display.setCursor(x2, y2);
    display.print(countText);
  }
  while (display.nextPage());
  // and put the display to sleep,
  // this should help prevent burn in.
  display.hibernate();
  delay(500);
}
