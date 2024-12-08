
/**
 * @file micropulser.ino
 * @brief Arduino Microsecond Pulse Generator
 * 
 * Generates precise microsecond pulses on digital output pins based on serial commands.
 * Supports single, periodic, test, and double pulse modes with user-defined parameters.
 * Supports interrupts on pin D2 and D3 to trigger short pulses on D4 and D5.
 * 
 * @author rsxy
 * @date 2024-12-07
 * 
 * @note Modify the 'delta_t' constant for timing calibration if necessary.
 *  
 * @version 0.3.1
 * 
 * Project URL: https://github.com/rsxy/micropulser
 *
 * MIT License
 * Copyright (c) 2024 rsxy
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * micropulser - A simple µs pulse generator for Arduino (tested on nano)
 * 
 * Can generate single pulses with microsecond duration on multiple pins,
 * double pulses with varying duration and gap
 * permanently set pin state to HIGH (1) or LOW (0)
 * periodic pulses of defined length and less strict periodicity
 * set of test pulses of different lengths
 * 
 * Serial commands:
 * pulse <int pinID>  <int pulseN> <int length> <int gap>
 * double <int pinID> <int pulselen> <int pulsegap> <int pinID2> <int pulselen2>\n"
 * setpin <int pinID> <int pinstate = 0 or 1>\n"
 * periodic <int pinID> <int length> <int gap>
 * stop 
 * test    // using pin 2, 3, 4, 5 for 1, 2, 5 µs pulses
 * 
 */

// Four output pins for pulses, GND is next to pin D2:

const int delta_t = 0;   // extra delay in µs to get correct timing
                        // with oscilloscope, all pulses were ~1 µs too short!
                        // with delta_t = 1 µs, ~ 0.5 µs too long!
                        // for now, just set to 0

const int inputPin1 = 2; // Interrupt input 1
const int inputPin2 = 3; // Interrupt input 2

const int testpulses [] = {1, 2, 5};   // µs for testpulses

String inputString = "";  // a string to hold incoming data
String runmode = "";      // a string set mode of operation

const char helpstr[] = "Arduino micropulser: Commands with integer values only, duration in µs:\n" 
"    pulse <int pinID> <int pulseN> <int pulselen> <int pulsegap>\n"
"    double <int pinID> <int pulselen> <int pulsegap> <int pinID2> <int pulselen2>\n"
"    periodic <int pinID> <int pulselen> <int pulsegap>\n"
"    setpin <int pinID> <int pinstate = 0 or 1>\n"
"    test\n"
"    stop\n";

// Global variable for identification string
const char* softwareVersion = "micropulser v0.3.0";

//char inputString[40];
bool stringComplete = false;  // whether the input string is complete, after receiving \n
char mesg[128];  // for using sprintf:  C string / char array

volatile unsigned int pinID = 0;      // pin ID for pulses
volatile unsigned int pulselen = 0;   // pulse length in µs
volatile unsigned int pulseN = 0;     // number of pulses
volatile unsigned int pulsegap = 0;   // gap between pulses, in µs
volatile unsigned int pinID2 = 0;     // pin ID2 for double pulses
volatile unsigned int pulselen2 = 0;  // pulse length2 in µs
volatile unsigned int pinstate = 0;   // pinstate for setpin 

// Interrupt-triggered outputs: Keep separate from other (double) pulse config:
volatile unsigned int outputPin1 = 4;   // pinID for interrupt input 1
volatile unsigned int outputPin2 = 5;   // pinID for interrupt input 2
volatile unsigned int outputLen1 = 3;   // pulse length in µs
volatile unsigned int outputLen2 = 3;   // pulse length in µs


void setup() {
    // Configure input pins
    pinMode(inputPin1, INPUT);
    pinMode(inputPin2, INPUT);

    // Attach interrupts
    attachInterrupt(digitalPinToInterrupt(inputPin1), handleInterrupt1, RISING);
    attachInterrupt(digitalPinToInterrupt(inputPin2), handleInterrupt2, RISING);
    
    // Define output pins D4 - D10, set low
    // Pins 3, 5, 6, 9, 10 can be used as analog out (PWM) using analogWrite(pin, value), value 0..255
    for (pinID=4; pinID<=10; pinID++){
        pinMode(pinID, OUTPUT);
        digitalWrite(pinID, LOW);
    }

    Serial.begin(115200);
    inputString.reserve(64);   // 64 bytes should be sufficient!
}

// Interrupt Service Routine for inputPin1
// Generate single short pulse on outputPin1
void handleInterrupt1() {
    cli();    // disable interrupts
    PIND=(1<<outputPin1);
    delayMicroseconds(pulselen+delta_t);
    PIND=(1<<outputPin1);
    sei();    // enable interrupts
    Serial.println("OK - Interrupt D2, pulse done!");             
}


// Interrupt Service Routine for inputPin2
// Generate single short pulse on outputPin2
void handleInterrupt2() {
    cli();    // disable interrupts
    PIND=(1<<outputPin2);
    delayMicroseconds(pulselen2+delta_t);
    PIND=(1<<outputPin2);
    sei();    // enable interrupts
    Serial.println("OK - Interrupt D3, pulse done!");             
}





void loop() {
    if (runmode == "pulse"){    // parse parameters and reset input string 
        cli();    // disable interrupts
        for (int i=0; i<pulseN; i++){    // generate N individual pulses
            PIND=(1<<pinID);
            delayMicroseconds(pulselen+delta_t);
            PIND=(1<<pinID);
            delayMicroseconds(pulsegap);   // spacing between pulses
        }  
        sei();    // enable interrupts
        Serial.println("OK - Pulse(s) done!");             
        // reset run mode to wait for new command!
        runmode = "";       
    }   // end "pulse"


    else if (runmode == "double"){     // double pulses, parameters have been parsed already in SerialEvent
        cli();    // disable interrupts
        PIND=(1<<pinID);
        delayMicroseconds(pulselen+delta_t);
        PIND=(1<<pinID);
        delayMicroseconds(pulsegap);   // spacing between pulses  
        PIND=(1<<pinID2);
        delayMicroseconds(pulselen2+delta_t);
        PIND=(1<<pinID2);
        sei();    // enable interrupts
        // reset run mode to wait for new command!
        runmode = "";
    }   // end "double"
  
    else if (runmode == "periodic"){     // periodic pulses until new command
          // parameters have been parsed already in SerialEvent
          // here, just send a single pulse, then in next iteration of main loop
          // this is for sending short pulses (strict timing) in less strict periods (milliseconds to seconds)
          cli();    // disable interrupts
          PIND=(1<<pinID);
          delayMicroseconds(pulselen+delta_t);
          PIND=(1<<pinID);
          sei();    // enable interrupts

          delayMicroseconds(pulsegap);   // spacing between pulses    
    }   // end "periodic"

    else if (runmode == "test"){        // test pulses (different lengths) until new command
        // now generate some test pulses, on all four channels!
        cli();    // disable interrupts
        for (int pin=2; pin<6; pin++){    // loop over all four pins # 2, 3, 4, 5
            for (int testpulse : testpulses) { // for each element in the array
                PIND=(1<<pin);
                delayMicroseconds(testpulse+delta_t);
                PIND=(1<<pin);
                delayMicroseconds(10);   // spacing between pulses
            }
        }
        sei();    // enable interrupts
        delay(1000);   // 1000 ms, resulting in approx 1 Hz
    }   // end "test"

}   // end main loop     
  

 

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.

  Commands terminate with "\n", run mode is parsed from first word,
  other parameters as neeeded, all separated by space.

  rsxy: borrowed from some example (serial string parsing?), 
        extended for immediate parsing and setting run mode.
        One-off commands could be immediately executed, though.
*/

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set parse run mode
    // and set stringComplete = True
    
    if (inChar == '\n') {
      if (inputString.startsWith("pulse ")){  
          // parse parameters here, just use in upper loop!
          // periodic 3 4 10 100   # pin 3, 4 pulses, 10 µs pulses, 100 µs wait time 
         int preceding_space = -1;
          long para[4];  // array for four parameters
          for(int i=0; i<4; i++) { //read four parameters
            preceding_space = inputString.indexOf(' ',preceding_space+1);
            if(preceding_space<0){
              Serial.println(F("Error: command is pulse <int pinID>  <int pulseN> <int pulselen> <int pulsegap>"));
              break;
            }
            para[i] = inputString.substring(preceding_space+1).toInt();
          }
          pinID = para[0];      // pin ID for pulses
          pulseN = para[1];     // number of pulses
          pulselen = para[2];   // pulse length in µs
          pulsegap = para[3];   // gap between pulses, in µs
          sprintf(mesg, "OK - Pulse mode: pinID: %d, # of pulses: %d, pulse length: %d µs, pulse gap: %d µs", pinID, pulseN, pulselen, pulsegap);
          Serial.println(mesg);
          runmode = "pulse";
      }

      else if (inputString.startsWith("double ")){  
          // parse parameters here, just use in upper loop!
          // double 2 5 10 3 20 will generate a pulse on pin D2 (5 µs), wait for 10 µs and generate another pulse on pin D3 (20 µs)
          int preceding_space = -1;
          long para[5];  // array for five parameters
          for(int i=0; i<5; i++) { //read four parameters
            preceding_space = inputString.indexOf(' ',preceding_space+1);
            if(preceding_space<0){
              Serial.println(F("Error: Command is 'periodic <int pinID> <int pulselen> <int pulsegap>'"));
              break;
            }
            para[i] = inputString.substring(preceding_space+1).toInt();
          }
          pinID = para[0];     // pin ID for pulses
          pulselen = para[1];   // pulse length in µs
          pulsegap = para[2];   // gap between pulses, in µs
          pinID2 = para[3];     // pin ID2 for pulses
          pulselen2 = para[4];   // pulse length2 in µs
  
          sprintf(mesg, "OK - double pulse mode: pinID: %d, pulse length: %d µs, pulse gap: %d µs, pinID2: %d, pulse length2: %d µs", 
                                                pinID, pulselen, pulsegap, pinID2, pulselen2);
          Serial.println(mesg);
          runmode = "double";
      }

      else if (inputString.startsWith("periodic ")){  
          // parse parameters here, just use in upper loop!
          // periodic 3 10 100   # pin 3, 10 µs pulses, 100 µs wait time 
          int preceding_space = -1;
          long para[3];  // array for four parameters
          for(int i=0; i<3; i++) { //read four parameters
            preceding_space = inputString.indexOf(' ',preceding_space+1);
            if(preceding_space<0){
              Serial.println(F("Error: Command is 'periodic <int pinID> <int pulselen> <int pulsegap>'"));
              break;
            }
            para[i] = inputString.substring(preceding_space+1).toInt();
          }
          pinID = para[0];     // pin ID for pulses
          pulselen = para[1];   // pulse length in µs
          pulsegap = para[2];   // gap between pulses, in µs
  
          sprintf(mesg, "OK - Starting with periodic mode: pinID: %d, pulse length: %d µs, pulse gap: %d µs", pinID, pulselen, pulsegap);
          Serial.println(mesg);
          runmode = "periodic";
      }

      else if (inputString.startsWith("setpara ")){  
          // This could be a generic function for all kinds of parameters
          // Now: set pulse length and outputID for interrupt input D2 and D3
          int pin, ID, len;
          sscanf(inputString.c_str(), "setpara %d %d %d", &pin, &ID, &len);
          if (pin == inputPin1) {
              outputPin1 = ID;
              outputLen1 = len;
              Serial.print("Pulse length for Pin ");
              Serial.print(inputPin1);
              Serial.print(" set to ");
              Serial.println(outputLen1);
          } else if (pin == inputPin2) {
              outputPin2 = ID;
              outputLen2 = len;
              Serial.print("Pulse length for Pin ");
              Serial.print(inputPin2);
              Serial.print(" set to ");
              Serial.println(outputLen2);
          } else {
              Serial.println("Invalid pin. Use 2 or 3.");
          }
   
      } 
      
      else if (inputString.startsWith("setpin ")){  
        // Set a single pin to 1 = HIGH or 0 = LOW, execute immediately
        int preceding_space = -1;
        long para[2];  // array for four parameters
        for(int i=0; i<2; i++) { //read two parameters
          preceding_space = inputString.indexOf(' ',preceding_space+1);
          if(preceding_space<0){
            Serial.println(F("Error: Command is 'setpin <int pinID> <int pinstate>'"));
            break;
          }
          para[i] = inputString.substring(preceding_space+1).toInt();
        }
        pinID = para[0];     // pin ID for pulses
        pinstate = para[1];   // pulse length in µs

        if (pinstate == 0) {
            digitalWrite(pinID, LOW);
        } else if (pinstate == 1) {
            digitalWrite(pinID, HIGH);
        } else {
            Serial.println("Error: pinState must be 0 (LOW) or 1 (HIGH).");
        }
        sprintf(mesg, "OK - Set pin %d to digital value %d", pinID, pinstate);
      }
      
      else if (inputString.startsWith("setpwm ")){  
        // Set a single pin to 0..255
        int preceding_space = -1;
        long para[2];  // array for four parameters
        for(int i=0; i<2; i++) { //read two parameters
          preceding_space = inputString.indexOf(' ',preceding_space+1);
          if(preceding_space<0){
            Serial.println(F("Error: Command is 'setpwm <int pinID> <int value>'"));
            break;
          }
          para[i] = inputString.substring(preceding_space+1).toInt();
        }
        pinID = para[0];     // pin ID for pulses
        pinstate = para[1];   // PWM value, 0..255

        // Check if the value is within the valid PWM range (0-255)
        if (value < 0) {
          value = 0; // If value is below 0, set it to 0
        } else if (value > 255) {
          value = 255; // If value is above 255, set it to 255
        }
      
        // Set the PWM value on the specified pin
        analogWrite(pin, value);
        }
        sprintf(mesg, "OK - Set pin %d to PWM value %d", pinID, pinstate);
      }
      
      else if (inputString.startsWith("test")){  
          Serial.println("OK - Test mode, sending pulses of 1, 2, 5 us length!");     
          runmode = "test";
      }
      
      else if (inputString.startsWith("help")){      
          Serial.println(helpstr);     
          runmode = "";  // empty string, so no specific mode will be entered
      }
      else if (inputString.startsWith("version") || inputString.startsWith("*IDN?")) {
          Serial.println(softwareVersion);
      }
      else if (inputString.startsWith("stop")){  
          Serial.println("OK - Stop mode, waiting for new command..");     
          runmode = "";  // empty string, so no specific mode will be entered
      }

      
      // now a few dedicated functions for testing, to eliminate any possible overhead
      else if (inputString.startsWith("singlefix1usD2")){ 
          // hard-coded pinID and duration, fastest exectution possible 
          Serial.println("OK - singlefix1usD2");     
          cli(); // disable interrupts
          PIND = (1 << 2);
          delayMicroseconds(1);
          PIND = (1 << 2);
          sei(); // enable interrupts
          runmode = "";  // empty string, so no specific mode will be entered
      }
      else if (inputString.startsWith("singlefix2usD2")){ 
          // hard-coded pinID and duration, fastest exectution possible 
          Serial.println("OK - singlefix2usD2");     
          cli(); // disable interrupts
          PIND = (1 << 2);
          delayMicroseconds(2);
          PIND = (1 << 2);
          sei(); // enable interrupts
          runmode = "";  // empty string, so no specific mode will be entered
      }
      else if (inputString.startsWith("singlefix3usD2")){ 
          // hard-coded pinID and duration, fastest exectution possible 
          Serial.println("OK - singlefix3usD2");     
          cli(); // disable interrupts
          PIND = (1 << 2);
          delayMicroseconds(3);
          PIND = (1 << 2);
          sei(); // enable interrupts
          runmode = "";  // empty string, so no specific mode will be entered
      }
      else if (inputString.startsWith("singlefix4usD2")){ 
          // hard-coded pinID and duration, fastest exectution possible 
          Serial.println("OK - singlefix4usD2");     
          cli(); // disable interrupts
          PIND = (1 << 2);
          delayMicroseconds(4);
          PIND = (1 << 2);
          sei(); // enable interrupts
          runmode = "";  // empty string, so no specific mode will be entered
      }
      else if (inputString.startsWith("singlefix5usD2")){ 
          // hard-coded pinID and duration, fastest exectution possible 
          Serial.println("OK - singlefix5usD2");     
          cli(); // disable interrupts
          PIND = (1 << 2);
          delayMicroseconds(5);
          PIND = (1 << 2);
          sei(); // enable interrupts
          runmode = "";  // empty string, so no specific mode will be entered
      }
      else if (inputString.startsWith("singlefix6usD2")){ 
          // hard-coded pinID and duration, fastest exectution possible 
          Serial.println("OK - singlefix6usD2");     
          cli(); // disable interrupts
          PIND = (1 << 2);
          delayMicroseconds(6);
          PIND = (1 << 2);
          sei(); // enable interrupts
          runmode = "";  // empty string, so no specific mode will be entered
      }
       
      else{
          Serial.println("Warning: Command could not be parsed! Try 'test' for 1, 2, 5 µs test pulse");
          // don't change run mode, just go on with whatever is was
          // runmode = "";
      }
          
      inputString = "";
     
      stringComplete = true;
    }
  }
}
