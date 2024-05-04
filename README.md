# micropulser
Microsecond pulse generator for Arduino.

Serial interface for generating short (µs) pulses on digital output pins.
Can generate single pulses and pulse trains with rather strict timing, periodic pulses with less strict timing (see below).
Test pulse pattern with 1, 2, 5 µs pulse width, periodically on pin D2, D3, D4, D5.

## Serial commands 
Parameters are separated by single space, termination character is "\n", 115200 baud.

Syntax:

`pulse <int pinID> <int pulseN> <int pulselen> <int pulsegap>`
* generate single or multiple pulses, then go to idle state, wait for new command.
* `<int pinID>`: digital output pin
* `<int pulseN>`: Number of periodic pulses to be generated
* `<int pulselen>`: Length of pulse(s) in microseconds (µs)
* `<int pulsegap>`: Gap between pulses, in microseconds (µs)

`double <int pinID> <int pulselen> <int pulsegap> <int pinID2> <int pulselen2>`
* generate two pulses on two individual pins, with defined duration and delay

`periodic <int pinID> <int pulselen> <int pulsegap>`
* generate single pulses in infinite loop; 
* pause timing is not strict, arduino runtime loop overhead is added to pulsegap.

`setpin <int pinID> <int pinstate = 0 or 1>`
* set individual pins to LOW (0) or HIGH (1)

`test`
* send periodic pattern of test pulses, i.e. 1, 2, 5 µs pulses with 10 µs gap, 
* using pin D2, D3, D4, D5, 
* then 1000 ms wait time (+ overhead) until next iteration

`stop`
* stop pulse generation, go to idle mode, waiting for new command

`help`
* output help string

`version` or `*IDN?`
* output version string


## Testing
Compile and upload to Arduino board. Open "Tools -> Serial Monitor" and enter commands, e.g. `test`.
Connect pin(s) to oscilloscope and watch the pulses!

Tested so far with an Arduino nano clone and pin 2 - 5, serial connection to Raspberry Pi.

## Timing accuracy
As to now, the program consits of simple loops and instructions to set the indiviual pins low or high.
At low pulse durations, the microcontroller comes to its limits. In addition, the program structure (loop, serial interrupt) will add additonal overlay.

My first tests with the Arduino nano clone show that the minium pulse length is about 2.16 µs (for both 0 and 1 µs set values), the minium gap between pulses 2.8 µs (for 0 and 1 µs set value), for pulse trains (multiple pulses using the `pulse` command).

From what I have seen so far, the timing is quite reproducible, the absolute pulse length and gap differs from the set value.
Depending on the application, this might be tolarable and can be corrected to some extend by the triggering commands.

Below are some screenshots and graphs, as tested with an Agilent DSO1072B oscilloscope.

![test___0](https://user-images.githubusercontent.com/5566528/236679824-01d12e9e-3edd-4111-a472-7d71afaea327.png)

Test pulses with 1, 2, 5, 10 µs nominal width, resulting in 0.8, 1.8, 4.8 and 9.8 µs actual pulse width.

![p23_1_1_](https://user-images.githubusercontent.com/5566528/236680531-9e409941-7eba-40f5-80a1-f75a326d67ef.png)

Three test pulses of 1 µs nominal width and 1 µs gap, resulting in 2.16 µs actual pulse and 2.8 µs gap width. Seemingly, the different 'for' loop contributes to the overhead. For short single pulses, the effect is the same - here, a dedicated function would probably work better!


## Contributing

This is my first public project for quite some time, please feel free to contribute or drop me a note! :-)

## To do
* more oscilloscope pictures
* compare fixed functions to using variables
* configurable delta parameter

