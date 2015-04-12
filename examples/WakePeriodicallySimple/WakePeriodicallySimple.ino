// 
// A Simple example showing how to set the RTC alarm pin to wake up the Arduino
// and then power up the Raspberry Pi at set intervals.
//
// What it does
// ============
// It uses the Alarm function on the RTC which is a 24 bit down counter
// clocked every second. Once the count reaches 0 - Boom! No, actually 
// it triggers the alarm pin, then reloads and starts all over again.
//
// The Arduino, meantime, powers off the RPi and goes into a deep sleep
// consuming virtually no power at all. When the Alarm goes off, it wakes up,
// then wakes the RPi. The RPi thens boots up and performs the task that it
// was awoken from it's slumber to perform. Once this is complete, ideally the 
// RPi should issue a:
//     sudo shutdown -h now
// and shut itself down - safe in the knowledge that it's performed it's job
// well.
// All the while the Arduino is watching and monitoring. If it detects that the 
// RPi has finished it's business, it cuts the last of the power and goes back into
// a deep sleep once more, awaiting the next alarm.
//
// Options
// ========
// - It the "Configuration" section below setup the Interval and Time outs that
//   you need.
// - The actual time on the RTC doesn't really matter for this example as it is
//   working on intervals. If you want the exact time you can sync it first 
//   to the RPi: http://spellfoundry.com/sleepy-pi/accessing-real-time-clock-raspberry-pi/
//   If you just want a close approximation, then use the code in the "Setup"
//   section which will set the clock to the time that the code was compiled which
//   will be about 10-20 seconds behind the actual time.
// - The code is sprinkled with messages than can be removed or commented out 
//   when running and reduce code size. You can only really see them via the development 
//   cable: http://spellfoundry.com/products/sleepy-pi-programming-adapter/


// +++++ Configuration ++++++++++
// Adjust the values below to suit your needs
// ...Interval - the time between RPi Power-ups
#define TIME_INTERVAL_SECONDS    0
#define TIME_INTERVAL_MINUTES    15    // i.e RPi powers up every 15 Mins   
#define TIME_INTERVAL_HOURS      0
#define TIME_INTERVAL_DAYS       0

// ...Failsafe time. If your RPi hasn't
// ...shutdown, we'll shut it down anyway
// ...Note: the Failsafe should be less than your
// ........ startup Interval!!
#define RPI_POWER_TIMEOUT_MS     90000    // in ms - so this is 90 seconds

// +++++ End Configuration +++++++

// **** INCLUDES *****
#include "SleepyPi.h"
#include <Time.h>
#include <LowPower.h>
#include <DS1374RTC.h>
#include <Wire.h>

typedef enum {
   ePI_OFF = 0,
   ePI_BOOTING,
   ePI_ON,
   ePI_SHUTTING_DOWN,
   ePI_SHUTDOWN
}ePISTATE;

ePISTATE  rpiState;
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
  tmElements_t powerUpInterval, compileTime;
   
  // Configure "Standard" LED pin
  pinMode(LED_PIN, OUTPUT);		
  digitalWrite(LED_PIN,LOW);		// Switch off LED

  SleepyPi.enablePiPower(false); 
  SleepyPi.enableExtPower(false);
  
  rpiState = ePI_OFF;

  // initialize serial communication: In Arduino IDE use "Serial Monitor"
  Serial.begin(9600);
  Serial.println("Starting, but I'm going to go to sleep for a while...");  
  
  // get the date and time the compiler was run
  if (getDate(__DATE__,compileTime) && getTime(__TIME__,compileTime)) 
  {
    // and configure the RTC with this info
    SleepyPi.setTime(compileTime);
  }
  
  powerUpInterval.Second = TIME_INTERVAL_SECONDS; 
  powerUpInterval.Minute = TIME_INTERVAL_MINUTES;
  powerUpInterval.Hour   = TIME_INTERVAL_HOURS;
  powerUpInterval.Day    = TIME_INTERVAL_DAYS;
  Serial.print("Alarm Interval: ");
  printTime(powerUpInterval,false);

  SleepyPi.setAlarm(powerUpInterval);
  SleepyPi.enableWakeupAlarm();
  delay(1000);  
}

void loop() 
{
    unsigned long timeNowMs, timeStartMs;
    tmElements_t  currentTime; 
    bool pi_running;
  
    // Allow wake up alarm to trigger interrupt on falling edge.
    attachInterrupt(0, alarm_isr, FALLING);		// Alarm pin
    SleepyPi.ackAlarm();
    
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(0); 
    
    SleepyPi.enablePiPower(true);   
    Serial.print("I've Just woken up: ");
    SleepyPi.readTime(currentTime);
    printTime(currentTime,true); 
    
    digitalWrite(LED_PIN,HIGH);		// Switch on LED
    delay(250);  
    digitalWrite(LED_PIN,LOW);		// Switch off LED
   
    rpiState = ePI_BOOTING; 
    
    // The RPi is now awake. Wait for it to shutdown or
    // Force it off after a set timeout.  
    timeStartMs = timeNowMs = millis();
    while (((timeNowMs - timeStartMs) < RPI_POWER_TIMEOUT_MS) && rpiState != ePI_SHUTDOWN)
    {
         pi_running = SleepyPi.checkPiStatus(false);
         if(pi_running == true)
         {
            Serial.print("+");
            rpiState = ePI_ON;
            delay(200);      // milliseconds   
         }
         else
         {
           if(rpiState == ePI_BOOTING)
           { 
              Serial.print("."); 
              delay(500);      // milliseconds   
           }
           else {
              Serial.println(); 
              Serial.println("RPi not running...");
              rpiState = ePI_SHUTDOWN;               
           }
         }
         timeNowMs = millis();
    }
    // Did a timeout occur?
    if((timeNowMs - timeStartMs) >= RPI_POWER_TIMEOUT_MS)
    {
       Serial.println(); 
       Serial.print("TimeOut! At: ");
       if (SleepyPi.readTime(currentTime)) 
       {
          printTime(currentTime,false); 
       }
       // Manually Shutdown the Rpi
       SleepyPi.piShutdown(true);      
       SleepyPi.enableExtPower(false);
       rpiState = ePI_OFF; 
            
    }
    else
    {
       Serial.println(); 
       Serial.println("RPi Shutdown, I'm going back to sleep..."); 
       SleepyPi.piShutdown(true);      
       SleepyPi.enableExtPower(false);
       rpiState = ePI_OFF;       
    } 
   
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
