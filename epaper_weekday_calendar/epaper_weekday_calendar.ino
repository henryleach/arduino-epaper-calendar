// Simple Daily Calendar on epaper screen with Arduino
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
char dateTimeString[26]; // longest is "31 September 2000, 23:59" plus terminator.
char weekDayString[10]; // Wednesday plus terminator
char tempTime[5];
char timeString[6];
time_t t;


void setup()
{

  myRTC.begin();
  setSyncProvider(myRTC.get);   // the function to get the time from the RTC

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
  displayWeekday();
}

// Displays the day of the week in the top half of the screen
void displayWeekday() {

  display.init();

  display.setRotation(3); //0 is 'portrait'

  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  u8g2Fonts.setFont(u8g2_font_logisoso50_tr); // Just narrow enough for 'Wednesday'.

  uint16_t x = ((display.width() - u8g2Fonts.getUTF8Width(dayStr(weekday(t)))) / 2);  // centres the text
  uint16_t y = 62;
  display.setFullWindow();
  display.firstPage();
  do // update the whole screen
  {
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.print(dayStr(weekday(t)));
  }
  while (display.nextPage());
  display.hibernate();
  

}

// Displays the day, month, year and time in the bottom half of the screen
void displayDateTime() {

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

  strcpy(dateTimeString, itoa(day(t), tempTime, 10));
  strcat(dateTimeString, " ");
  strcat(dateTimeString, monthStr(month(t))); 
  strcat(dateTimeString, " ");
  strcat(dateTimeString, itoa(year(t), tempTime, 10));
  strcat(dateTimeString, " ");
  strcat(dateTimeString, timeString);

  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  u8g2Fonts.setFont(u8g2_font_logisoso20_tr); // Just narrow enough for 'Wednesday'.

  uint16_t x = ((display.width() - u8g2Fonts.getUTF8Width(dateTimeString)) / 2);  // centres the text

  uint16_t y = 110;
  display.setPartialWindow(0, 80, display.width(), display.height());
  display.firstPage();
  do // update the bottom half of the screen
  {
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.print(dateTimeString);
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
    displayWeekday();
  }
  displayDateTime();
  
  delay(500); // if this isn't here the arduino seems to fall asleep before finishing the last job
  
  setAlarm();

  //Allow wake up pin to trigger on interrupt low.
  attachInterrupt(digitalPinToInterrupt(2), alarmIsr, FALLING);

  //power down
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  //Wakes up here!

  //disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(2));
  if (myRTC.alarm(DS3232RTC::ALARM_1)){
    //pass, alarm reset.
  }

}

void setAlarm() {
  //setAlarm(alarmType, seconds, minutes, hours, daydate)
  myRTC.setAlarm(DS3232RTC::ALM1_MATCH_SECONDS, 0, 0, 0, 1); // starts on next whole minute
  //myRTC.setAlarm(DS3232RTC::ALM1_MATCH_HOURS, 0, 0, 0, 1); // set for midnight.

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
