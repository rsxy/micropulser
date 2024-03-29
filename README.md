# micropulser

Microsecond pulse generator for Arduino.

This project involves an Arduino program capable of generating microsecond pulses on digital output pins. 
The program allows users to control the pulse generation through serial commands, 
enabling various modes such as single pulses, periodic pulses, test pulses, and double pulses on different pins.

The code can generate single pulses and pulse trains with rather strict timing, periodic pulses with less strict timing (see below).
Test pulse pattern with 1, 2, 5, 10 µs (or alternatively 1, 2, 4 µs) pulse width, periodically on pin D2, D3, D4, D5.

Also, individual pins can be just set HIGH to LOW  - boring!

By default, pins #2 - #10 are set as digital outputs and can be addressed by commands.


## Serial command interface
Parameters are separated by single space, termination character is "\n", 115200 baud.
There will be some inherent communication latency until the command gets actually executed.

## Syntax

### pulse
`pulse <pinID> <pulseN> <pulselen> <pulsegap>`

Generates a specified number of pulses on a pin.

- `pinID`: Pin number to output pulses.
- `pulseN`: Number of pulses to generate.
- `pulselen`: Length of each pulse in microseconds.
- `pulsegap`: Gap between consecutive pulses in microseconds.

Example: Using pin3, generate 5 pulses of 10 microseconds each with a gap of 50 microseconds:
`pulse 3 5 10 50`


### periodic
`periodic <pinID> <pulselen> <pulsegap>`

Generates pulses at regular intervals on a pin.

- `pinID`: Pin number to output pulses.
- `pulselen`: Length of the pulse in microseconds.
- `pulsegap`: Interval between pulses in microseconds.

Example: Using pin4, to start periodic pulses of 10 microseconds with a gap of 500 microseconds:

  `periodic 4 10 500` - to stop pulsing send `stop`


### doublepulse
`doublepulse <pinID> <pulselen> <gap> <pinID2> <pulselen2>`

Generates a double pulse sequence using two different pins. Can be used to trigger some event and a camera (or vice versa) with a defined delay.

- `pinID`: First pin number for pulse output.
- `pulselen`: Length of the pulse on the first pin in microseconds.
- `gap`: Gap between the pulses from the two pins in microseconds.
- `pinID2`: Second pin number for pulse output.
- `pulselen2`: Length of the pulse on the second pin in microseconds.

Example: To generate a double pulse sequence on pins 2 and 5 with respective pulse lengths of 10 and 20 microseconds and a gap of 100 microseconds:

  `doublepulse 2 10 100 5 20`

### test
`test` : Generates a sequence of test pulses of varying lengths (1, 2, 5, 10 µs) on all configured pins.

`test2` : Generates a sequence of test pulses of varying lengths (1, 2, 4 µs) on all configured pins.

### stop
`stop`

Stops any ongoing pulse generation and waits for a new command.

### setpin
`setpin <pinID> <pinsate>` : Set pin to LOW (pinstate = 0) or HIGH (pinstate = 1)

Example: Set pin D9 to HIGH: `setpin 9 1`



### help
`help`

## Testing
Compile and upload to Arduino board. Open "Tools -> Serial Monitor" and enter commands, e.g. `help` or `test`.
Connect pin(s) to oscilloscope and watch the pulses!


Tested so far with an Arduino nano clone and pin 2 - 5, serial connection to Raspberry Pi or a Windows PC using LabVIEW.

## Timing accuracy
As to now, the program consits of simple loops and instructions to set the indiviual pins low or high.
At low pulse durations, the microcontroller comes to its limits. In addition, the program structure (loop, serial interrupt) will add additonal overlay.

My first tests with the Arduino nano clone show that the minium pulse length is about 2.16 µs (for both 0 and 1 µs set values), the minium gap between pulses 2.8 µs (for 0 and 1 µs set value), for pulse trains (multiple pulses using the `pulse` command).

From what I have seen so far, the timing is quite reproducible, the absolute pulse length and gap differs from the set value.
Depending on the application, this might be tolarable and can be corrected to some extend by the delta_t parameter.

Below are some screenshots and graphs, as tested with an Agilent DSO1072B oscilloscope.

![test___0](https://user-images.githubusercontent.com/5566528/236679824-01d12e9e-3edd-4111-a472-7d71afaea327.png)

Test pulses with 1, 2, 5, 10 µs nominal width, resulting in 0.8, 1.8, 4.8 and 9.8 µs actual pulse width.

![p23_1_1_](https://user-images.githubusercontent.com/5566528/236680531-9e409941-7eba-40f5-80a1-f75a326d67ef.png)

Three test pulses of 1 µs nominal width and 1 µs gap, resulting in 2.16 µs actual pulse and 2.8 µs gap width. Seemingly, the different 'for' loop contributes to the overhead. For short single pulses, the effect is the same - here, a dedicated function would probably work better!


## Contributing

This is my first public project for quite some time, please feel free to contribute or drop me a note! :-)

## To do
* more testing and oscilloscope pictures

