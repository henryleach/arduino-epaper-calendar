#include <U8g2_for_Adafruit_GFX.h>
#include <u8g2_fonts.h>

// Simple Clock with date on epaper screen with Arduino
// www.henryleach.com

#include <DS3232RTC.h>      // https://github.com/JChristensen/DS3232RTC
// select the display class and display driver class in the following file (new style):

#include <GxEPD2_BW.h> // including both doesn't use more code or ram
#include <GxEPD2_3C.h> // including both doesn't use more code or ram
#include <Fonts/FreeSansBold12pt7b.h>
#include "GxEPD2_display_selection_new_style.h"
#include <LowPower.h> // https://github.com/rocketscream/Low-Power

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;  // font constructor

DS3232RTC myRTC;  // create the RTC object
const uint8_t wakeUpPin(2); // connect Arduino pin D2 to RTC's SQW pin.
char dateString[18]; //longest is "31 September 2000" plus terminator.
char timeString[6];
char yearString[5];
char tempTime[3];
time_t t;


void setup()
{
  Serial.begin(115200);
  myRTC.begin();
  setSyncProvider(myRTC.get);   // the function to get the time from the RTC
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }

  u8g2Fonts.begin(display); //connect the u8g2


  // initialize the alarms to known values, clear the alarm flags, clear the alarm interrupt flags
  myRTC.setAlarm(DS3232RTC::ALM1_MATCH_DATE, 0, 0, 0, 1);
  myRTC.setAlarm(DS3232RTC::ALM2_MATCH_DATE, 0, 0, 0, 1);
  myRTC.alarm(DS3232RTC::ALARM_1);
  myRTC.alarm(DS3232RTC::ALARM_2);
  myRTC.alarmInterrupt(DS3232RTC::ALARM_1, false);
  myRTC.alarmInterrupt(DS3232RTC::ALARM_2, false);
  myRTC.squareWave(DS3232RTC::SQWAVE_NONE);

  //configure an interrupt on the falling edge from SQN pin
  pinMode(wakeUpPin, INPUT_PULLUP);

  t = myRTC.get(); //get the time from the RTC

  displayDate();

}

// Displays the date in the bottom half of the screen
// does a complete screen refresh
void displayDate()
{
  display.init();

  //Now print the date in smaller text below.
  //Create the date string
  //write the date in first
  itoa(day(t), dateString, 10); //base 10
  itoa(year(t), yearString, 10);
  strcat(dateString, " ");
  strcat(dateString, monthStr(month(t)));
  strcat(dateString, " ");
  strcat(dateString, yearString);
  Serial.println(dateString);
  display.setRotation(1); //0 is 'portrait'
  display.setFont(&FreeSansBold12pt7b); //This almost fills the screen with "Wednesday"
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(dateString, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;  // centres the text
  uint16_t y = 100; //bottom
  // covers bottom half
  display.setFullWindow();
  display.firstPage();
  do // Print the upper part of the screen
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(dateString);
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
  //Only numbers and symbols to save space.https://github.com/olikraus/u8g2/wiki/fntlist99#50-pixel-height
  u8g2Fonts.setFont(u8g2_font_logisoso50_tn);
  
  uint16_t x = ((display.width() - u8g2Fonts.getUTF8Width(timeString)) / 2);  // centres the text
  Serial.println(x);
  uint16_t y = 60; //top half

  display.setPartialWindow(0, 0, display.width(), display.height() / 2);
  display.firstPage();
  do // Print the upper part of the screen
  {
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.print(timeString);
  }
  while (display.nextPage());
  display.hibernate();

}

void loop()
{

  t = myRTC.get(); //get the time from the RTC

  if (minute(t) == 0){
    // Refresh the display completely
    // every hour, hopefully reduces ghosting and burn in
    // also updates the date at midnight
    displayDate();
  }
  displayTime();

  delay(500); // if this isn't here the arduino seems to fall asleep before finishing the last job

  setAlarm();

  //Allow wake up pin to trigger on interrupt low.
  attachInterrupt(digitalPinToInterrupt(2), alarmIsr, FALLING);

  //power down
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  //Wakes up here!

  //disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(2));
  Serial.println("Woke Up!");
  if (myRTC.alarm(DS3232RTC::ALARM_1)) {
    Serial.println("Alarm 1 triggered, and reset");
  }

}

void setAlarm() {
  //setAlarm(alarmType, seconds, minutes, hours, daydate)
  myRTC.setAlarm(DS3232RTC::ALM1_MATCH_SECONDS, 0, 0, 0, 1); // starts on next whole minute
  myRTC.alarm(DS3232RTC::ALARM_1);
  myRTC.alarmInterrupt(DS3232RTC::ALARM_1, true);
}

void alarmIsr()
{
  Serial.println("Interrupt Triggered");
}
