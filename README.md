# NCM109_NixieClock
GRA &amp; AFCH Nixie Tubes Clock custom alternative firmware (NCM109)

Visual Studio Code + platformIO project

The original firmware has a bug related to the dimming of the blue channel due to the use of the Tone library which interferes with the PWM on pin D3.

In this version I have chosen to use the TimerFreeTone library.
Unfortunately this library is blocking and therefore when the melody is played the nixie are piloted in a "strange way". 
The visual effect obtained match with the rhythm of the music, so I decided to keep.

I started by correcting this bug, but at the end of story I decided to rewrite everything from scratch.

I have strongly limited the use of the String class as much as possible;
Eliminated the code for the Dallas DS18B20 temperature sensor using the internal one of the DS3231;
Added a library for the management of the RGB Led in a more flexible and user-configurable way (is possible set the light intensity and the fading cycle speed);
Added several transition effects like text scrolling or countdown in step 9-> 0 (todo: ghost effect);
