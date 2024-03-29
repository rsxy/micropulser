/**
 * @file micropulser.ino
 * @brief Arduino Microsecond Pulse Generator
 * 
 * Generates precise microsecond pulses on digital output pins based on serial commands.
 * Supports single, periodic, test, and double pulse modes with user-defined parameters.
 * 
 * @author rsxy
 * @date 2024-03-29
 * 
 * @note Modify the 'delta_t' constant for timing calibration if necessary.
 * 
 * @note Code refactored using ChatGPT 4.0
 * 
 * @version 0.2
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


// Constants

// Global variable for identification string
const char* softwareVersion = "micropulser v0.2";
const int delta_t = 0;  // Calibration delay in microseconds
const int testpulses[] = {1, 2, 5, 10};  // Test pulse durations in microseconds
const int testpulses2[] = {1, 2, 4};

// Pin definitions
const int pinA = 2;
const int pinB = 3;
const int pinC = 4;
const int pinD = 5;
const int pinE = 6;  // New pins for double pulse or setpin feature
const int pinF = 7;  
const int pinG = 8;  
const int pinH = 9;  
const int pinI = 10;  



// Global variables
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;     // whether the string is complete
char mesg[256];                  // buffer to hold messages
String runmode = "";             // Operation mode
int pinID = 0;                   // pin ID for pulses
int pulselen = 0;                // pulse length in microseconds
int pinID2 = 0;                  // pin ID for second double pulse
int pulselen2 = 0;               // second pulse length in microseconds
int pulseN = 1;                  // number of pulses
int pulsegap = 0;                // gap between pulses in microseconds
int pinstate = 0;                // 0 for LOW, 1 for HIGH

char helpstr[] = "Arduino micropulser: Commands with integer values only, duration in µs:\n" 
"    pulse <int pinID> <int pulseN> <int pulselen> <int pulsegap>\n"
"    periodic <int pinID> <int pulselen> <int pulsegap>\n"
"    doublepulse <int pinID> <int pulselen> <int pulsegap> <int pinID2> <int pulselen2>\n"
"    setpin <int pinID> <int pinstate>\n"
"    test\n"
"    stop\n";



void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    
    if (inChar == '\n') {
      parseCommand(inputString);
      inputString = "";
      stringComplete = true;
    }
  }
}


void setup() {
  pinMode(pinA, OUTPUT);
  digitalWrite(pinA, LOW);
  pinMode(pinB, OUTPUT);
  digitalWrite(pinB, LOW);
  pinMode(pinC, OUTPUT);
  digitalWrite(pinC, LOW);
  pinMode(pinD, OUTPUT);
  digitalWrite(pinD, LOW);
  pinMode(pinE, OUTPUT);
  digitalWrite(pinE, LOW);
  pinMode(pinF, OUTPUT);
  digitalWrite(pinF, LOW);
  pinMode(pinG, OUTPUT);
  digitalWrite(pinG, LOW);
  pinMode(pinH, OUTPUT);
  digitalWrite(pinH, LOW);
  pinMode(pinI, OUTPUT);
  digitalWrite(pinI, LOW);

  Serial.begin(115200);
  inputString.reserve(64);
}

void loop() {
  if (stringComplete) { // if the string is complete
    if (runmode == "pulse") {
      generatePulses(pinID, pulseN, pulselen, pulsegap);
      Serial.println("OK - Pulse(s) done!");
      runmode = ""; // reset run mode
    } else if (runmode == "periodic") {
      generateSinglePulse(pinID, pulselen);
      delayMicroseconds(pulsegap); // spacing between pulses
    } else if (runmode == "doublepulse") {
      doublePulse(pinID, pulselen, pulsegap, pinID2, pulselen2);
      Serial.println("OK - Double pulse done!");
      runmode = ""; // reset run mode
    } else if (runmode == "test") {
      generateTestPulses();
    } else if (runmode == "test2") {
      generateTestPulses2();
    }
    stringComplete = false; // ready to receive new data
  }
}

/**
 * @brief Generates a sequence of two pulses on different pins with a gap in between.
 *
 * @param pin1 First pin number to output the pulse.
 * @param len1 Pulse length for the first pin in microseconds.
 * @param gap Gap between the pulses from the two pins in microseconds.
 * @param pin2 Second pin number to output the pulse.
 * @param len2 Pulse length for the second pin in microseconds.
 */
// Function to handle the double pulse mode
void doublePulse(int pin1, int len1, int gap, int pin2, int len2) {
  cli(); // disable interrupts
  PIND = (1 << pin1);
  delayMicroseconds(len1 + delta_t);
  PIND = (1 << pin1);
  delayMicroseconds(gap);
  PIND = (1 << pin2);
  delayMicroseconds(len2 + delta_t);
  PIND = (1 << pin2);
  sei(); // enable interrupts
}

void generatePulses(int pin, int numPulses, int pulseLength, int gap) {
  cli(); // disable interrupts
  for (int i = 0; i < numPulses; i++) {
    PIND = (1 << pin);
    delayMicroseconds(pulseLength + delta_t);
    PIND = (1 << pin);
    delayMicroseconds(gap); // spacing between pulses
  }
  sei(); // enable interrupts
}

void generateSinglePulse(int pin, int pulseLength) {
  cli(); // disable interrupts
  PIND = (1 << pin);
  delayMicroseconds(pulseLength + delta_t);
  PIND = (1 << pin);
  sei(); // enable interrupts
}

void generateTestPulses() {
  cli(); // disable interrupts
  for (int pin = pinA; pin <= pinD; pin++) { // loop over pins
    for (int pulse : testpulses) {
      PIND = (1 << pin);
      delayMicroseconds(pulse + delta_t);
      PIND = (1 << pin);
      delayMicroseconds(10); // spacing between pulses
    }
  }
  sei(); // enable interrupts
  delay(1); // 1 ms delay
}

void generateTestPulses2() {
  cli(); // disable interrupts
  for (int pin = pinA; pin <= pinD; pin++) { // loop over pins
    for (int pulse : testpulses2) {
      PIND = (1 << pin);
      delayMicroseconds(pulse + delta_t);
      PIND = (1 << pin);
      delayMicroseconds(10); // spacing between pulses
    }
  }
  sei(); // enable interrupts
  delay(100); // 100 ms delay
}

/**
 * @brief Sets the specified pin to the given state (LOW or HIGH).
 *
 * @param pinID The pin number to be set.
 * @param pinState The state to set the pin (0 for LOW, 1 for HIGH).
 */
void setPin(int pinID, int pinState) {
  if (pinState == 0) {
    digitalWrite(pinID, LOW);
  } else if (pinState == 1) {
    digitalWrite(pinID, HIGH);
  } else {
    Serial.println("Error: pinState must be 0 (LOW) or 1 (HIGH).");
  }
}


void parseCommand(String command) {
  if (command.startsWith("pulse ")) {
    parsePulseCommand(command);
  } else if (command.startsWith("periodic ")) {
    parsePeriodicCommand(command);
  } else if (command.startsWith("doublepulse")) {
    parseDoublePulseCommand(command);
  } else if (command.startsWith("setpin ")) {
    parseSetPinCommand(command);
  } else if (command.startsWith("test2")) {
    Serial.println("OK - Test mode 2, sending pulses of varying length!");
    runmode = "test2";
  } else if (command.startsWith("test")) {
    Serial.println("OK - Test mode, sending pulses of varying length!");
    runmode = "test";
  } else if (command.startsWith("help")) {
    Serial.println(helpstr);
    runmode = "";
  } else if (command.equalsIgnoreCase("version") || command.equalsIgnoreCase("*IDN?")) {
    Serial.println(softwareVersion);
  } else if (command.startsWith("stop")) {
    Serial.println("OK - Stop mode, waiting for new command..");
    runmode = "";
  } else {
    Serial.println("Warning: Command could not be parsed! Try 'test' for 1, 2, 5, 10 µs test pulses");
  }
}

void parsePulseCommand(String command) {
  long params[4];
  if(parseParameters(command, "pulse", params, 4)){
    pinID = params[0];
    pulseN = params[1];
    pulselen = params[2];
    pulsegap = params[3];
    sprintf(mesg, "OK - Pulse mode: pinID: %d, # of pulses: %d, pulse length: %d µs, pulse gap: %d µs", pinID, pulseN, pulselen, pulsegap);
    Serial.println(mesg);
    runmode = "pulse";
  }
}

void parsePeriodicCommand(String command) {
  long params[3];
  if(parseParameters(command, "periodic", params, 3)){
    pinID = params[0];
    pulselen = params[1];
    pulsegap = params[2];
    sprintf(mesg, "OK - Starting with periodic mode: pinID: %d, pulse length: %d µs, pulse gap: %d µs", pinID, pulselen, pulsegap);
    Serial.println(mesg);
    runmode = "periodic";
  }
}

void parseDoublePulseCommand(String command) {
  long params[5];
  if(parseParameters(command, "doublepulse", params, 5)){
    int pin1 = params[0];
    int len1 = params[1];
    int gap = params[2];
    int pin2 = params[3];
    int len2 = params[4];
    sprintf(mesg, "OK - Double pulse mode: pin1: %d, len1: %d µs, gap: %d µs, pin2: %d, len2: %d µs", pin1, len1, gap, pin2, len2);
    Serial.println(mesg);
    runmode = "doublepulse";
  }
}

/**
 * @brief Parses the set pin command and calls the setPin function with the parameters.
 *
 * @param command The command string containing the parameters for setting a pin.
 */
void parseSetPinCommand(String command) {
  long params[2]; // Expecting two parameters: pinID and pinState
  if (parseParameters(command, "setpin", params, 2)) {
    int pinID = params[0];
    int pinState = params[1];
    setPin(pinID, pinState);
    sprintf(mesg, "OK - Pin %d set to %d", pinID, pinState);
    Serial.println(mesg);
  }
}


bool parseParameters(String command, const char* mode, long* params, int numParams) {
  int spaceIndex = -1;
  for(int i = 0; i < numParams; i++) {
    spaceIndex = command.indexOf(' ', spaceIndex+1);
    if(spaceIndex < 0) {
      Serial.print(F("Error: Command format is '"));
      Serial.print(mode);
      Serial.println(" <parameters>' with the correct number of parameters.");
      return false;
    }
    params[i] = command.substring(spaceIndex+1).toInt();
  }
  return true;
}
