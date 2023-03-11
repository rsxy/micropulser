# micropulser
Microsecond pulse generator for Arduino.

Serial interface for generating short (µs) pulses on digital output pins.
Can generate single pulses and pulse trains with rather strict timing, periodic pulses with less strict timing.
Test pulse pattern with 1, 2, 5, 10 µs pulse width, peridically on pin D2, D3, D4, D5.

Serial commands (parameters separated by single space, termination character is "\n"):

pulse <int pinID>  <int pulseN> <int pulselen> <int pulsegap>
* generate single or multiple pulses, then go to idle state, waiting for new command.
* <int pinID>: digital output pin
* <int pulseN>: Number operiodic <int pin> <int length> <int gap>
f pulses to be generated
* <int pulselen>: Length of pulse(s) in microseconds (µs)
* <int pulsegap>: Gap between pulses, in microseconds (µs)

periodic <int pinID> <int pulselen> <int pulsegap>
* generate single pulses in infinite loop; pause timing is not strict, arduino runtime loop overhead is added to pulsegap.

test
* send periodic pattern of test pulses, i.e. 1, 2, 5, 10 µs pulses with 10 µs gap, on pin D2, D3, D4, D5, then 1 ms wait time (+ overhead) until next iteration

stop
* stop pulse generation, go to idle mode, waiting for new command



