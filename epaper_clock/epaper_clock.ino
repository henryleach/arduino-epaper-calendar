// Simple Clock with date on epaper screen with Arduino
// www.henryleach.com

#include <DS3232RTC.h> // https://github.com/JChristensen/DS3232RTC
#include <LowPower.h>  // https://github.com/rocketscream/Low-Power
#include <U8g2_for_Adafruit_GFX.h> // https://github.com/olikraus/U8g2_for_Adafruit_GFX
#include <u8g2_fonts.h>
// https://github.com/ZinggJM/GxEPD2
#include <GxEPD2_BW.h> // including both doesn't use more code or ram
#include <GxEPD2_3C.h> // including both doesn't use more code or ram
// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;  // font constructor
DS3232RTC myRTC;  // create the RTC object

const uint8_t wakeUpPin(2); // connect Arduino pin D2 to RTC's SQW pin.
const uint8_t dstPin(4); // connect to GND to add 1 hour as DST.
char dateString[18]; // longest is "31 September 2000" plus terminator.
char timeString[6];
char yearString[5];
char tempTime[3];
time_t t;


void setup()
{

  myRTC.begin();
  setSyncProvider(myRTC.get); // the function to get the time from the RTC

  u8g2Fonts.begin(display); // connect the u8g2

  // initialize the alarms to known values, clear the alarm flags, clear the alarm interrupt flags
  myRTC.setAlarm(DS3232RTC::ALM1_MATCH_DATE, 0, 0, 0, 1);
  myRTC.setAlarm(DS3232RTC::ALM2_MATCH_DATE, 0, 0, 0, 1);
  myRTC.alarm(DS3232RTC::ALARM_1);
  myRTC.alarm(DS3232RTC::ALARM_2);
  myRTC.alarmInterrupt(DS3232RTC::ALARM_1, false);
  myRTC.alarmInterrupt(DS3232RTC::ALARM_2, false);
  myRTC.squareWave(DS3232RTC::SQWAVE_NONE);

  // configure an interrupt on the falling edge from SQN pin
  pinMode(wakeUpPin, INPUT_PULLUP);
  pinMode(dstPin, INPUT_PULLUP);

  getDstTime();
  displayDate();

}

// Displays the date in the bottom half of the screen
// and does a complete screen refresh
void displayDate()
{
  display.init();

  itoa(day(t), dateString, 10); // base 10
  itoa(year(t), yearString, 10);
  
  strcat(dateString, " ");
  strcat(dateString, monthStr(month(t)));
  strcat(dateString, " ");
  strcat(dateString, yearString);
  
  display.setRotation(3); //0 is 'portrait'

  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  u8g2Fonts.setFont(u8g2_font_logisoso20_tr);
  
  uint16_t x = ((display.width() - u8g2Fonts.getUTF8Width(dateString)) / 2);  // centres the text
  uint16_t y = 110; //bottom
  // covers bottom half
  display.setFullWindow();
  display.firstPage();
  do // update the whole screen
  {
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.print(dateString);
  }
  while (display.nextPage());
  display.hibernate();
}

// Displays the time in the top half of the screen, as a partial refresh
void displayTime()
{
  
  itoa(hour(t), tempTime, 10);
  if (hour(t) < 10) {
    // zero pad
    strcpy(timeString, "0");
    strcat(timeString, tempTime);
  } else {
    strcpy(timeString, tempTime);
  }

  strcat(timeString, ":");

  itoa(minute(t), tempTime, 10);
  if (minute(t) < 10) {
    // zero pad
    strcat(timeString, "0");
    strcat(timeString, tempTime);
  } else {
    strcat(timeString, tempTime);
  }

  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  // Only numbers and symbols to save space.https://github.com/olikraus/u8g2/wiki/fntlist99#50-pixel-height
  u8g2Fonts.setFont(u8g2_font_logisoso50_tn);
  
  uint16_t x = ((display.width() - u8g2Fonts.getUTF8Width(timeString)) / 2); // centres the text
  uint16_t y = 62; //top half, depends on font

  display.setPartialWindow(0, 0, display.width(), display.height() / 2);
  display.firstPage();
  do // Update the upper part of the screen
  {
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.print(timeString);
  }
  while (display.nextPage());
  display.hibernate();

}

void loop()
{

  getDstTime();

  if (minute(t) == 0){
    // Refresh the display completely
    // every hour, hopefully reduces ghosting and burn in
    // also updates the date at midnight
    displayDate();
  }
  displayTime();

  delay(500); // if this isn't here the arduino seems to fall asleep before finishing the last job

  setAlarm();

  // Allow wake up pin to trigger on interrupt low.
  attachInterrupt(digitalPinToInterrupt(2), alarmIsr, FALLING);

  // Power down
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  // Wakes up here!

  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(2));

  if (myRTC.alarm(DS3232RTC::ALARM_1)) {
    //also clears alarm flag
  }

}

void setAlarm() {
  // setAlarm(alarmType, seconds, minutes, hours, daydate)
  myRTC.setAlarm(DS3232RTC::ALM1_MATCH_SECONDS, 0, 0, 0, 1); // starts on next whole minute
  myRTC.alarm(DS3232RTC::ALARM_1);
  myRTC.alarmInterrupt(DS3232RTC::ALARM_1, true);
}

void getDstTime() {
  // Get the time, and adjust for DST if pin D4 is connected to GND
  t = myRTC.get(); // get the time from the RTC

  if (digitalRead(dstPin) == LOW){
    t+=3600; // Add 1 hour in seconds
  }
  
}

void alarmIsr()
{
  // Need a function for the interrupt, but doesn't have to do anything.
}
