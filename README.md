SleepyPi
========

Arduino library for controlling the [Sleepy Pi] Power Management board for Raspberry Pi.

After downloading name the folder 'Sleepy Pi' and install in the Arduino libraries folder. On the Raspberry Pi running Raspbian this will be within the sketchbook folder at */home/pi/sketchbook/libraries*.

**NOTE**: This library **uses / inherits** the following two libraries which must also be installed in */home/pi/sketchbook/libraries*:

 - [ds1374RTC]
      - [time]
 - [lowpower]

if you use wakeup on pin changes other than the INT0 & INT1 pins then you can use this library: [pinchangeint]


[Sleepy Pi]: http://spellfoundry.com/sleepy-pi/
[ds1374RTC]: https://github.com/SpellFoundry/DS1374RTC.git
[lowpower]: https://github.com/rocketscream/Low-Power
[time]: http://playground.arduino.cc/Code/time
[pinchangeint]: https://code.google.com/p/arduino-pinchangeint/
