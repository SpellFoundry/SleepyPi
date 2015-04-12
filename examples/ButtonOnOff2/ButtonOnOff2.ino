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
#define kBUTTON_FORCEOFF_TIME_MS   8000


// States
typedef enum {
  eWAIT = 0,
  eBUTTON_PRESSED,
  eBUTTON_WAIT_ON_RELEASE,
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
eBUTTONSTATE   buttonState = eWAIT;
ePISTATE       pi_state = ePI_OFF;
bool state = LOW;
unsigned long  time, timePress;

void button_isr()
{
    // A handler for the Button interrupt.
    buttonPressed = true;
}

void alarm_isr()
{
    // A handler for the Alarm interrupt.
}


void setup()
{
  SleepyPi.simulationMode = false;  // Don't actually shutdown
  
  // Configure "Standard" LED pin
  pinMode(LED_PIN, OUTPUT);		
  digitalWrite(LED_PIN,LOW);		// Switch off LED

  SleepyPi.enablePiPower(false);  
  SleepyPi.enableExtPower(false);
  
  // Allow wake up triggered by button press
  attachInterrupt(1, button_isr, LOW);       // button pin  
}

void loop() 
{
    bool pi_running;
    unsigned long buttonTime;
 
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake button is pressed.
    // Once button is pressed stay awake - this allows the timer to keep running

    switch(buttonState)
    {
        case eWAIT:
             // Allow wake up alarm to trigger interrupt on falling edge.
            attachInterrupt(0, alarm_isr, FALLING);    // Alarm pin
            // Setup the Alarm Counter
            SleepyPi.enableWakeupAlarm();
            SleepyPi.setAlarm(5); // 2 Seconds or whatever.. 
            
            // Enter power down state with ADC and BOD module disabled.
            // Wake up when wake up pin is low or Alarm fired.
            SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
            // GO TO SLEEP....
            // ....
            // ....
            // I'm awake !!!      
            // What woke me up? Was it a button press or a scheduled wake?
            // Lets check on button press
           if(buttonPressed == false)
           {           
                // Was an alarm interrupt     
                // Do some general housekeeping
                // ...Check on our RPi
                digitalWrite(LED_PIN,HIGH);    // Flash LE
                pi_running = SleepyPi.checkPiStatus(false); 
                switch(pi_state)
                {            
                  case ePI_BOOTING:
                       // Check if we have finished booting
                       if(pi_running == true)
                       {
                           // We have booted up!
                           pi_state = ePI_ON;                  
                       }
                       else 
                       {
                           // Still not completed booting so lets carry on waiting
                           pi_state = ePI_BOOTING;                   
                       } 
                       break;
                  case ePI_ON:
                       // Check if it is still on?
                       if(pi_running == false)
                       {
                           // Shock horror! it's not running!!
                           // Assume it has been manually shutdown, so lets cut the power
                           // Force Pi Off               
                           SleepyPi.enablePiPower(false);
                           SleepyPi.enableExtPower(false);
                           pi_state = ePI_OFF;                           
                       }
                       else 
                       {
                           // Still on - all's well - keep this state
                           pi_state = ePI_ON;                   
                       } 
                       break;
                  case ePI_SHUTTING_DOWN:
                      // Is it still shutting down? 
                       if(pi_running == false)
                       {
                           // Finished a shutdown
                           // Force the Power Off               
                           SleepyPi.enablePiPower(false);
                           SleepyPi.enableExtPower(false);
                           pi_state = ePI_OFF;                           
                       }
                       else 
                       {
                           // Still shutting down - keep this state
                           pi_state = ePI_SHUTTING_DOWN;                   
                       } 
                      break;
                  case ePI_OFF:             
                  default:       // intentional drop thru
                     // RPi off, so we'll continue to wait
                     // for a button press to tell us to switch it on
                     delay(10);
                     pi_state = ePI_OFF;    
                     break; 
                }
                buttonState = eWAIT; // Loop back around and go to sleep again
                digitalWrite(LED_PIN,LOW);
           }
           else
           {
              buttonPressed = false;
              // This was a button press so change the button state (and stay awake)
              // Disable the alarm interrupt
              detachInterrupt(0);              
              // Disable external pin interrupt on wake up pin.
              detachInterrupt(1);
              buttonState = eBUTTON_PRESSED; 
           }
           break;
        case eBUTTON_PRESSED:
            buttonPressed = false;  
            timePress = millis();    // Log Press time                
            pi_running = SleepyPi.checkPiStatus(false);
            if(pi_running == false)
            {  
                // Switch on the Pi
                SleepyPi.enablePiPower(true);
                SleepyPi.enableExtPower(true);
                pi_state = ePI_BOOTING;   
            }          
            buttonState = eBUTTON_WAIT_ON_RELEASE;
            digitalWrite(LED_PIN,HIGH);         
            attachInterrupt(1, button_isr, HIGH);    // Will go high on release           
            break;
        case eBUTTON_WAIT_ON_RELEASE:
            if(buttonPressed == true)
            {
                detachInterrupt(1); 
                buttonPressed = false;                            
                time = millis();     //  Log release time
                buttonState = eBUTTON_RELEASED;
            }
            else
            {
                // Carry on waiting
                buttonState = eBUTTON_WAIT_ON_RELEASE;  
            }
            break;
        case eBUTTON_RELEASED:           
            pi_running = SleepyPi.checkPiStatus(false);
            if(pi_running == true)
            {
                // Check how long we have held button for
                buttonTime = time - timePress;
                if(buttonTime > kBUTTON_FORCEOFF_TIME_MS)
                {
                   // Force Pi Off               
                   SleepyPi.enablePiPower(false);
                   SleepyPi.enableExtPower(false);
                   pi_state = ePI_OFF;          
                } 
                else if (buttonTime > kBUTTON_POWEROFF_TIME_MS)
                {
                    // Start a shutdown
                    pi_state = ePI_SHUTTING_DOWN; 
                    SleepyPi.piShutdown(false);      // true
                    SleepyPi.enableExtPower(false);            
                } 
                else 
                { 
                     // Button not held off long - Do nothing
                } 
            } 
            else 
            {
                // Pi not running  
            }
            digitalWrite(LED_PIN,LOW);            
            attachInterrupt(1, button_isr, LOW);    // button pin
            buttonState = eWAIT;        
            break;
        default:
            break;
    }
}

