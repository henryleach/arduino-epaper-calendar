// Use a DS3231 real time clock to wake a sleeping Arduino
// www.henryleach.com

#include <DS3232RTC.h> // https://github.com/JChristensen/DS3232RTC
#include <LowPower.h> // https://github.com/rocketscream/Low-Power

DS3232RTC myRTC;  // create the RTC object
const uint8_t wakeUpPin(2); // connect Arduino pin Digital 2 to RTC's SQW pin.
//const time_t alarmInterval(10); //alarm interval in seconds

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

  setAlarm();

}

void loop()
{

  setSyncProvider(myRTC.get);
  
  time_t t = myRTC.get(); //get the time from the RTC
  char dateTime[40];
  char notDay[30];
  snprintf(notDay,
           sizeof(notDay),
           "%i %s %i %02i:%02i:%02i",
           day(t),
           monthStr(month(t)),
           year(t),
           hour(t),
           minute(t),
           second(t));

  strcpy(dateTime, dayStr(weekday(t)));
  strcat(dateTime, " ");
  strcat(dateTime, notDay);

  Serial.println(dateTime);
  delay(500); //if this isn't here the arduino seems to go to sleep before printing everything to serial

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

void setAlarm(){
  myRTC.setAlarm(DS3232RTC::ALM1_MATCH_SECONDS, 0, 0, 0, 1); // starts on next whole minute
  myRTC.alarm(DS3232RTC::ALARM_1);
  myRTC.alarmInterrupt(DS3232RTC::ALARM_1, true);
}

void alarmIsr()
{
  Serial.println("Interrupt Triggered");
}
