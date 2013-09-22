/*
 * SleepyPi.cpp - library for Sleepy Pi Board
  
  Copyright (c) Jon Watkins 2013
  http://spellfoundry.com

  This is a library of functions ofr use with the Sleepy Pi Power
  Management board for Raspberry Pi. 

  NOTE
  =======
  This library is dependent on the following libraries which must also
  be present in the libraries directory:

  - lowpower (http://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library/)
  			 (http://github.com/rocketscream/Low-Power)
  
  - ds1374RTC (http://github.com/SpellFoundry/DS1374RTC.git)


  License
  =======
  The library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Releases
  ========
  V1_0 - 23 Aug 2013 - Initial release

 */

#include "SleepyPi.h"

#define ENABLE_PI_PWR_PIN	16				// PC2 - O/P take high to enable the RaspPi - Active High
#define ENABLE_EXT_PWR_PIN	4				// PD4 - O/P take high to enable the External Supplies
#define CMD_PI_TO_SHDWN_PIN	17				// PC3 - 0/P Handshake to request the Pi to shutdown - Active high
#define PI_IS_RUNNING		7				// PD7 - I/P Handshake to show that the Pi is running - Active High
#define V_SUPPLY_PIN		20				// I/P - A/I Supply monitoring pin
#define POWER_BUTTON_PIN	3				// PD3 - I/P User Power-on Button (INT1) - Active Low
#define ALARM_PIN			2				// PD2 - I/P Pin that pulses when the alarm has expired (INT0) - Active Low

// Constructors
SleepyPiClass::SleepyPiClass()
{
	RTCConfig_t rtc_config; 

	// Reset flags
	simulationMode = false;
	simPiOn		   = false;
	pi_running	   = false;
	power_on	   = false;
	ext_power_on   = false;

	// **** Configure Power supplies ***
	// ...Configure Pi Power
	pinMode(ENABLE_PI_PWR_PIN, OUTPUT);		
	SleepyPiClass::enablePiPower(false);	// ***** RasPi is Off at Startup ***** //

	// ...Configure Ext Expansion Power
	pinMode(ENABLE_EXT_PWR_PIN, OUTPUT);		
	SleepyPiClass::enableExtPower(false);	// ***** Expansion Power is Off at Startup ***** //
											// Change if RasPi is to be powered on at boot
	// **** Configure I/O *****
	// ...Configure Pi Shutdown handshake to PI
	pinMode(CMD_PI_TO_SHDWN_PIN, OUTPUT);     
	digitalWrite(CMD_PI_TO_SHDWN_PIN,LOW);	// Don't command to shutdown
  
	// ...Configure Pi Shutdown handshake from PI - goes high when Pi is running
	pinMode(PI_IS_RUNNING, INPUT);    

	// ...Initialize the button pin as a input: also can be used as an interrupt INT1
	pinMode(POWER_BUTTON_PIN, INPUT);
 
	// ...Initialize the alarm as a input: also can be used as an interrupt INT0
	pinMode(ALARM_PIN, INPUT);

#ifdef DEBUG_MESSAGES
	int handShake;
	// initialize serial communication:
	Serial.begin(9600);

	// Display Status
	//read the state of the Pi
	handShake = digitalRead(PI_IS_RUNNING);
	if(handShake > 0){
	 Serial.println("Handshake I/P high");    
	}
	else {
	 Serial.println("Handshake I/P low");        
	}
	Serial.println("Pi Shutdown O/P low");
	Serial.println("Pi Power O/P low");

#endif
  
}
  
// PUBLIC FUNCTIONS
/* **************************************************+

	--enableExtPower

	Switch on / off the power to the external 
	expansion pins.

+****************************************************/
void SleepyPiClass::enableExtPower(bool enable)
{
	if(simulationMode == true){
		if(enable == true){
			// Turn on the External Expansion power
			// digitalWrite(ENABLE_EXT_PWR_PIN,HIGH);
			ext_power_on = true;			
		}
		else {
			// Turn off the External Expansion power
			// digitalWrite(ENABLE_EXT_PWR_PIN,LOW);
			ext_power_on = false;			
		}
	}
	else {
		if(enable == true){
			// Turn on the External Expansion power
			digitalWrite(ENABLE_EXT_PWR_PIN,HIGH);
			ext_power_on = true;
		}
		else {
			// Turn off the External Expansion power
			digitalWrite(ENABLE_EXT_PWR_PIN,LOW);
			ext_power_on = false;
		}	
	}
	return;
}
/* **************************************************+

	--enablePiPower

	Switch on / off the power to the Raspberry Pi.

+****************************************************/
void SleepyPiClass::enablePiPower(bool enable)
{
	if(simulationMode == true){
		if(enable == true){
			// Turn on the Pi
			// digitalWrite(ENABLE_PI_PWR_PIN,HIGH);
			power_on = true;
		}
		else {
			// Turn off the Pi
			// digitalWrite(ENABLE_PI_PWR_PIN,LOW);
			power_on = false;
		}
	}
	else {
		if(enable == true){
			// Turn on the Pi
			digitalWrite(ENABLE_PI_PWR_PIN,HIGH);
			power_on = true;
		}
		else {
			// Turn off the Pi
			digitalWrite(ENABLE_PI_PWR_PIN,LOW);
			power_on = false;
		}
	}
	return;
}

/* **************************************************+

	--enableWakeupAlarm

	Setup and enable the RTC Alarm. This will output
	a pulse on the /INT pin of the RTC which is recieved
	as a low going pulse on the INT0 pin of the ATMEGA

+****************************************************/
bool SleepyPiClass::enableWakeupAlarm(void)
{
	RTCConfig_t	rtc_config;

	rtc_config.disableOsc = false;	    // Enable / Disable the RTC OScillator
	rtc_config.enableCTR = true;	    // Enable / Disable the Watchdog or Alarm Counters
	rtc_config.CTRType = eALARM;		// Select either the Watchdog or the Alarm as the Counter
	rtc_config.enableSQW = false;	    // Enable / Disable the square wave output
	rtc_config.WDToutput = eINT_PIN;	// Select where the reset pulse goes when the WDT expires
	rtc_config.SQWRate = e1Hz;			// Selects the frequency of the square wave output			
	rtc_config.enableAlarmInt = true;	// Enable / Disable the Alarm for setting the INT pin when it expires
	
	return DS1374RTC::setConfig(rtc_config);	

	// TODO do we need to attach an interrupt here?

}
/* **************************************************+

	--StartPiShutdown

	Set the handshake lines so that the Raspberry Pi
	is given notice to begin shutting down.

+****************************************************/
void SleepyPiClass::StartPiShutdown(void)
{
	if(simulationMode == true){
		// Serial.println("StartPiShutdown()");
		power_on = false;
		ext_power_on = false;
	}
	else {
		// Command the Sleepy Pi to shutdown
		digitalWrite(CMD_PI_TO_SHDWN_PIN,HIGH);	
	}

	return;
}

/* **************************************************+

	--checkPiStatus

	Monitor the handshake lines and determine whether
	the Raspberry Pi is running or not.

	Option to Force a shutdown i.e. remove power to
	the Raspberry Pi if it is not handshaking.

+****************************************************/

bool SleepyPiClass::checkPiStatus(bool forceShutdownIfNotRunning)
{
	int	handShake;
	
	if(simulationMode == true){	
		handShake = power_on;
		pi_running = true;
	}
	else {
		handShake = digitalRead(PI_IS_RUNNING);		
	}	

	if(handShake > 0) {
		// RasPi is still running
		pi_running = true;
		return true;
	}
	else{
		// Pi not handshaking - either booting or manually shutdown
		if(forceShutdownIfNotRunning == true){
			// Pi not running - has it been on?
			if(pi_running == true){
				// Pi has been running and now isn't
				// so cut the power
				SleepyPiClass::enablePiPower(false);
				pi_running = false;
			}
		}
		return false;
	}

}
/* **************************************************+

	--piShutdown

	Set the Handshake lines to command a shutdown of
	the Raspberry Pi. Then wait for the Raspberry Pi
	to shutdown by monitoring the handshake line that 
	indicates that the Raspberry Pi is running. Once
	the RPi has shutdown remove the power after a 
	small guard interval.

	Option to Force a shutdown i.e. remove power to
	the Raspberry Pi if it is not handshaking.

+****************************************************/
void SleepyPiClass::piShutdown(bool forceShutdown)
{
	int	handShake;
	unsigned long timeStart, timeNow, testTime;
	
	// Command the Sleepy Pi to shutdown
	if(simulationMode == true){
		// Serial.println("piShutdown()");
		// Switch off the Pi
		delay(5000);
		SleepyPiClass::enablePiPower(false);
	}
	else {

		digitalWrite(CMD_PI_TO_SHDWN_PIN,HIGH);		
		
		// Wait for the Pi to shutdown 
		timeStart = millis();
		testTime = 0;
		handShake = digitalRead(PI_IS_RUNNING);
		while((handShake > 0) && (testTime < kFAILSAFETIME_MS)){
			handShake = digitalRead(PI_IS_RUNNING);  
			delay(50);
			timeNow = millis();
			testTime = timeNow - timeStart;
		}
		// Switch off the Pi
		delay(5000); // delay to make sure the Pi has finished shutting down
		SleepyPiClass::enablePiPower(false);
	}

	return;
}


SleepyPiClass SleepyPi;
