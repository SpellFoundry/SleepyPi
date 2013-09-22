// 
// Simple example showing how to set the Sleepy Pi to wake on button press
// and then power up the Raspberry Pi. To switch the RPi off press the button
// again. If the button is held dwon the Sleepy Pi will cut the power to the
// RPi regardless of any handshaking.
//

// **** INCLUDES *****
#include "SleepyPi.h"
#include <Time.h>
#include <LowPower.h>
#include <DS1374RTC.h>
#include <Wire.h>


#define kBUTTON_POWEROFF_TIME_MS   2000
#define kBUTTON_FORCEOFF_TIME_MS   10000


// States
typedef enum {
  eWAIT = 0,
  eBUTTON_PRESSED,
  eBUTTON_HELD,
  eBUTTON_RELEASED
}eBUTTONSTATE;

typedef enum {
   ePI_OFF = 0,
   ePI_BOOTING,
   ePI_ON,
   ePI_SHUTTING_DOWN
}ePISTATE;

const int LED_PIN = 13;

volatile bool  buttonPressed = false;
eBUTTONSTATE   buttonState = eBUTTON_RELEASED;
ePISTATE       pi_state = ePI_OFF;
bool state = LOW;
unsigned long  time, timePress;

void button_isr()
{
    // A handler for the Button interrupt.
    buttonPressed = true;
}

//void alarm_isr()
//{
    // A handler for the Alarm interrupt.
//}


void setup()
{
  SleepyPi.simulationMode = false;  // Don't actually shutdown
  
  // Configure "Standard" LED pin
  pinMode(LED_PIN, OUTPUT);		
  digitalWrite(LED_PIN,LOW);		// Switch off LED

  SleepyPi.enablePiPower(false);  
  SleepyPi.enableExtPower(false);
  
   // Allow wake up triggered by button press or Wdt timeout 
  attachInterrupt(1, button_isr, LOW);    // button pin 
  
  // initialize serial communication: In Arduino IDE use "Serial Monitor"
  Serial.begin(9600);
  
  // SleepyPi.simulationMode = true;  // Don't actually shutdown
  
}

void loop() 
{
    bool pi_running;
  
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake button is pressed.
    // Once button is pressed stay awake.
    pi_running = SleepyPi.checkPiStatus(true);  // Cut Power if we detect Pi not running
    if(pi_running == false){ 
       SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    }
    
    // Button State changed
    if(buttonPressed == true){
        detachInterrupt(1);      
        buttonPressed = false;  
        switch(buttonState) { 
          case eBUTTON_RELEASED:
              // Button pressed 
              Serial.println("Button Pressed");             
              timePress = millis();             
              pi_running = SleepyPi.checkPiStatus(false);
              if(pi_running == false){  
                  // Switch on the Pi
                  Serial.println("Pi On");
                  SleepyPi.enablePiPower(true);
                  SleepyPi.enableExtPower(true);   
              }          
              buttonState = eBUTTON_PRESSED;
              digitalWrite(LED_PIN,HIGH);           
              attachInterrupt(1, button_isr, HIGH);                    
              break;
          case eBUTTON_PRESSED:
              // Button Released
              Serial.println("Button Released");
              unsigned long buttonTime;
              time = millis();
              buttonState = eBUTTON_RELEASED;
              pi_running = SleepyPi.checkPiStatus(false);
              if(pi_running == true){
                  // Check how long we have held button for
                  buttonTime = time - timePress;
                  if(buttonTime > kBUTTON_FORCEOFF_TIME_MS){
                     // Force Pi Off               
                     SleepyPi.enablePiPower(false);
                     SleepyPi.enableExtPower(false);
                     Serial.println("Force Off"); 
                     // digitalWrite(LED_PIN,LOW);             
                  } else if (buttonTime > kBUTTON_POWEROFF_TIME_MS){
                      // Start a shutdown
                      Serial.println("Pi Shutdown");
                      SleepyPi.piShutdown(true);
                      SleepyPi.enableExtPower(false);
                      // digitalWrite(LED_PIN,LOW);               
                  } else { 
                     // Button not held off long - Do nothing
                  } 
              } else {
                  // Pi not running  
                  Serial.println("Pi not running"); 
              }
              digitalWrite(LED_PIN,LOW);            
              attachInterrupt(1, button_isr, LOW);    // button pin       
              break;
           default:
              break;
        }                
    } 
}

