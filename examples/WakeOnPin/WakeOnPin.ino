// 
// Simple example showing how to wake up the Arduino from a falling
// edge on a GPIO pin and then power up the Raspberry Pi
//
// NOTE: This library uses the "pinchangeint" library, which can be downloaded from
// https://code.google.com/p/arduino-pinchangeint/

// **** INCLUDES *****
#include "SleepyPi.h"
#include <Time.h>
#include <LowPower.h>
#include <DS1374RTC.h>
#include <Wire.h>
#include <PinChangeInt.h>

// Optimisation defines
#define NO_PORTC_PINCHANGES
#define DISABLE_PCINT_MULTI_SERVICE
#define NO_PIN_STATE
#define NO_PIN_NUMBER

const int LED_PIN = 13;
const int WAKEUP_PIN = 8;	// Pin B0 - Arduino 8

void wakeup_isr()
{
    // Just a handler for the wakeup interrupt.
    // You could do something here
}

void setup()
{

  // Configure "Standard" LED pin
  pinMode(LED_PIN, OUTPUT);		
  digitalWrite(LED_PIN,LOW);		// Switch off LED
  
  // Configure "Wakeup" pin 
  pinMode(WAKEUP_PIN, INPUT);		// Set as input	
  digitalWrite(WAKEUP_PIN, HIGH);	// Enable internal pull-up

  // Set the initial Power to be off
  SleepyPi.enablePiPower(false);  
  SleepyPi.enableExtPower(false);
  
  // initialize serial communication: In Arduino IDE use "Serial Monitor"
  Serial.begin(9600);

}

void loop()
{

	// Attach WAKEUP_PIN to wakeup pi 
	PCintPort::attachInterrupt(WAKEUP_PIN, &wakeup_isr, FALLING);  

    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    
    // Disable external pin interrupt on WAKEUP_PIN pin.
	PCintPort::detachInterrupt(WAKEUP_PIN); 
    
    // Do something here
    // Example: Read sensor, data logging, data transmission.
    // SleepyPi.enablePiPower(true);	// Uncomment to power up the RaspPi
    
    Serial.println("I've Just woken up");
    digitalWrite(LED_PIN,HIGH);		// Switch on LED
    delay(250);  
    digitalWrite(LED_PIN,LOW);		// Switch off LED 

}

