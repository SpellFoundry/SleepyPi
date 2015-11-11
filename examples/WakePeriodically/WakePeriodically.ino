// 
// Simple example showing how to set the RTC alarm pin to wake up the Arduino
// and then power up the Raspberry Pi
//

// **** INCLUDES *****
#include "SleepyPi.h"
#include <Time.h>
#include <LowPower.h>
#include <DS1374RTC.h>
#include <Wire.h>

#define RPI_POWER_TIMEOUT_MS     6000    // in ms - so this is 6 seconds
#define TIME_INTERVAL_SECONDS    30
#define TIME_INTERVAL_MINUTES    0   
#define TIME_INTERVAL_HOURS      0
#define TIME_INTERVAL_DAYS       0

tmElements_t powerUpTime;

const int LED_PIN = 13;
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void alarm_isr()
{
    // Just a handler for the alarm interrupt.
    // You could do something here

}

void setup()
{ 
  // Configure "Standard" LED pin
  pinMode(LED_PIN, OUTPUT);		
  digitalWrite(LED_PIN,LOW);		// Switch off LED

  SleepyPi.enablePiPower(false); 
  SleepyPi.enableExtPower(false);

  // initialize serial communication: In Arduino IDE use "Serial Monitor"
  Serial.begin(9600);
  Serial.println("Starting, but I'm going to go to sleep for a while...");

  // get the date and time the compiler was run
  if (getDate(__DATE__,powerUpTime) && getTime(__TIME__,powerUpTime)) {
    // and configure the RTC with this info
    SleepyPi.setTime(powerUpTime);
    // Set our Initial start time
    powerUpTime.Second += TIME_INTERVAL_SECONDS;
    powerUpTime.Minute += TIME_INTERVAL_MINUTES; 
    powerUpTime.Hour   += TIME_INTERVAL_HOURS;
    powerUpTime.Day    += TIME_INTERVAL_DAYS;
    Serial.print("Initial Wakeup at: ");
    printTime(powerUpTime,false); 
    delay(1000); 
    SleepyPi.enableWakeupAlarm();
    SleepyPi.setAlarm(30);         
  }
}

void loop() 
{
    unsigned long timeNowMs, timeStartMs;
    tmElements_t  currentTime; 
    bool pi_running;
  
    // Allow wake up alarm to trigger interrupt on falling edge.
    attachInterrupt(0, alarm_isr, FALLING);		// Alarm pin
//    SleepyPi.enableWakeupAlarm();
      SleepyPi.ackAlarm();
//    SleepyPi.setAlarm(powerUpTime); 
//    SleepyPi.setAlarm(30);
    
    // +++++++++ Test +++++++++++++
    SleepyPi.readTime(currentTime);
    Serial.print("Current Time = ");
    printTime(currentTime,false); 
    SleepyPi.readAlarm(currentTime);
    Serial.print("Alarm Time = ");
    printTime(currentTime,false); 
    delay(1000);
    // ++++++++++++++++++++++++++++++
    
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(0); 
    
    SleepyPi.enablePiPower(true);   
    Serial.print("I've Just woken up: ");
    SleepyPi.readTime(currentTime);
    printTime(currentTime,false); 
    
    //Calculate next Power-up Time
    powerUpTime.Second += TIME_INTERVAL_SECONDS;
    powerUpTime.Minute += TIME_INTERVAL_MINUTES; 
    powerUpTime.Hour   += TIME_INTERVAL_HOURS;
    powerUpTime.Day    += TIME_INTERVAL_DAYS;   
    
    digitalWrite(LED_PIN,HIGH);		// Switch on LED
    delay(250);  
    digitalWrite(LED_PIN,LOW);		// Switch off LED 
    
    // The RPi is now awake. Wait for it to shutdown or
    // Force it off after a set timeout.  
    timeStartMs = timeNowMs = millis();
    while ((timeNowMs - timeStartMs) < RPI_POWER_TIMEOUT_MS)
    {
         pi_running = SleepyPi.checkPiStatus(true);
         if(pi_running == true)
         {
            Serial.print(".");
            delay(100);      // milliseconds   
         }
         timeNowMs = millis();
    }
    // Did a timeout occur?
    if((timeNowMs - timeStartMs) >= RPI_POWER_TIMEOUT_MS)
    {
       Serial.print("TimeOut!:");
       if (SleepyPi.readTime(currentTime)) 
       {
          printTime(currentTime,false); 
       }    
    }
    else
    {
       Serial.println("RPi Shutdown, I'm going back to sleep...");     
    } 
    SleepyPi.piShutdown(true);      
    SleepyPi.enableExtPower(false); 
    
}



bool printTime(tmElements_t tm, bool printDate)
{
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    if(printDate == true)
    {
      Serial.print(", Date (D/M/Y) = ");
      Serial.print(tm.Day);
      Serial.write('/');
      Serial.print(tm.Month);
      Serial.write('/');
      Serial.print(tmYearToCalendar(tm.Year));
    }
    Serial.println();   
}

bool getTime(const char *str, tmElements_t &time)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  time.Hour = Hour;
  time.Minute = Min;
  time.Second = Sec;
  return true;
}

bool getDate(const char *str, tmElements_t &time)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  time.Day = Day;
  time.Month = monthIndex + 1;
  time.Year = CalendarYrToTm(Year);
  return true;
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
