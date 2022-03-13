// Simple Daily Calendar on epaper screen with Arduino
// www.henryleach.com

#include <DS3232RTC.h>      // https://github.com/JChristensen/DS3232RTC
// select the display class and display driver class in the following file (new style):

#include <GxEPD2_BW.h> // including both doesn't use more code or ram
#include <GxEPD2_3C.h> // including both doesn't use more code or ram
#include <Fonts/FreeSansBold24pt7b.h> //this uses a lot of memory.
#include <Fonts/FreeSansBold12pt7b.h>
#include "GxEPD2_display_selection_new_style.h"

#include <LowPower.h> // https://github.com/rocketscream/Low-Power

DS3232RTC myRTC;  // create the RTC object
const uint8_t wakeUpPin(2); // connect Arduino pin 2 to RTC's SQW pin.
char dateString[18]; //longest is "31 September 2000" plus terminator.
char yearString[5];

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

  displayDayAndDate();
}

void displayDayAndDate()
{
  display.init();
  time_t t = myRTC.get(); //get the time from the RTC

  // First print the day of the week in large across the top
  display.setRotation(1); //0 is 'portrait'
  display.setFont(&FreeSansBold24pt7b); //This almost fills the screen with "Wednesday"
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(dayStr(weekday(t)), 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;  // centres the text
  uint16_t y = 50; //top half
  display.setFullWindow();
  display.firstPage();
  do // Print the upper part of the screen
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(dayStr(weekday(t)));
  }
  while (display.nextPage());

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

  display.setFont(&FreeSansBold12pt7b); //This almost fills the screen with "Wednesday"
  display.setTextColor(GxEPD_BLACK);
  display.getTextBounds(dateString, 0, 0, &tbx, &tby, &tbw, &tbh);
  x = ((display.width() - tbw) / 2) - tbx;  // centres the text
  y = 100; //bottom
  // covers bottom half
  display.setPartialWindow(0, display.height() / 2, display.width(), display.height());
  display.firstPage();
  do // Print the upper part of the screen
  {
    display.setCursor(x, y);
    display.print(dateString);
  }
  while (display.nextPage());
  display.hibernate();
}

void loop()
{

  displayDayAndDate();

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
  if (myRTC.alarm(DS3232RTC::ALARM_1)){
    Serial.println("Alarm 1 triggered, and reset");
  }

}

void setAlarm() {
  //setAlarm(alarmType, seconds, minutes, hours, daydate)
  //myRTC.setAlarm(DS3232RTC::ALM1_MATCH_SECONDS, 0, 0, 0, 1); // starts on next whole minute
  myRTC.setAlarm(DS3232RTC::ALM1_MATCH_HOURS, 0, 0, 0, 1); // set for midnight.

  myRTC.alarm(DS3232RTC::ALARM_1);
  myRTC.alarmInterrupt(DS3232RTC::ALARM_1, true);
}

void alarmIsr()
{
  Serial.println("Interrupt Triggered");
}
