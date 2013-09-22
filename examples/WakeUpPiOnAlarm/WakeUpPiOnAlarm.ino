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

const int LED_PIN = 13;

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

  // initialize serial communication: In Arduino IDE use "Serial Monitor"
  Serial.begin(9600);

}

void loop() 
{
    // Allow wake up alarm to trigger interrupt on falling edge.
    attachInterrupt(0, alarm_isr, FALLING);		// Alarm pin

    SleepyPi.enableWakeupAlarm();
    SleepyPi.setAlarm(10);              // in seconds
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(0); 
    
    // Do something here
    // Example: Read sensor, data logging, data transmission.
    SleepyPi.enablePiPower(true);
    
    Serial.println("I've Just woken up");
    digitalWrite(LED_PIN,HIGH);		// Switch on LED
    delay(250);  
    digitalWrite(LED_PIN,LOW);		// Switch off LED 

}
