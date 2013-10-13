/*
 * SleepyPi.h - library for Sleepy Pi power management board
 */

#ifndef SLEEPYPI_h
#define SLEEPYPI_h

#include "Arduino.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <DS1374RTC.h>
#include <Time.h>
#include <Wire.h>
#include <LowPower.h>

// #define DEBUG_MESSAGES

#define kFAILSAFETIME_MS		30000		// Failsafe shutdown time in milliseconds
#define kONBUTTONTIME_MS		1000		// 
#define kONBUTTONTIME_MS		3000		// 
#define kFORCEOFFBUTTONTIME_MS	1000

// library interface description
//class SleepyPiClass {
class SleepyPiClass : public DS1374RTC , public LowPowerClass {
  
	// user-accessible "public" interface
  public:
	bool	simulationMode;
	bool	power_on;
	bool	ext_power_on;
	bool	pi_running;

	// TODO Timer

    SleepyPiClass();		// Look at initialising the simulationMode from the constructor

	// Power On Off
	void enableExtPower(bool enable);
	void enablePiPower(bool enable);

	// Control
	void  StartPiShutdown(void);
	bool  checkPiStatus(bool forceShutdownIfNotRunning);
	void  piShutdown(bool forceShutdown);

	// Time

	// Want to own the Alarm routine so may make these virtual in the base class	

	// Wakeup
	bool enableWakeupAlarm(void);

  private:
    static bool exists;
	bool	simPiOn;

//	time_t wakeupTime;

};

extern SleepyPiClass SleepyPi;

#endif // SLEEPYPI_h
 
