

/*
 * micropulser - A simple µs pulse generator for Arduino (tested on nano)
 * 
 * Can generate single pulses with microsecond duration on multiple pins,
 * periodic pulses of defined length and less strict periodicity
 * set of test pulses of different lengths
 * 
 * Serial commands:
 * pulse <int pinID>  <int pulseN> <int length> <int gap>
 * periodic <int pinID> <int length> <int gap>
 * stop 
 * test    // using pin 2, 3, 4, 5
 * 
 */

// Four output pins for pulses, GND is next to pin D2:

const int delta_t = 0;   // extra delay in µs to get correct timing
                        // with oscilloscope, all pulses were ~1 µs too short!
                        // with delta_t = 1 µs, ~ 0.5 µs too long!
                        // for now, just set to 0

const int testpulses [] = {1, 2, 5, 10};   // µs for testpulses

String inputString = "";  // a string to hold incoming data
String runmode = "";      // a string set mode of operation

char helpstr[] = "Arduino micropulser: Commands with integer values only, duration in µs:\n" 
"    pulse <int pinID> <int pulseN> <int pulselen> <int pulsegap>\n"
"    periodic <int pinID> <int pulselen> <int pulsegap>\n"
"    test\n"
"    stop\n";


//char inputString[40];
bool stringComplete = false;  // whether the string is complete
char mesg[256];  // for using sprintf:  C string / char array

int pinID = 0;      // pin ID for pulses
int pulselen = 0;   // pulse length in µs
int pulseN = 0;     // number of pulses
int pulsegap = 0;   // gap between pulses, in µs

int pinA = 2;
int pinB = 3;
int pinC = 4;
int pinD = 5;

void setup() {
    // define pins, set low
    pinMode(pinA, OUTPUT);
    digitalWrite(pinA, LOW);

    pinMode(pinB, OUTPUT);
    digitalWrite(pinB, LOW);

    pinMode(pinC, OUTPUT);
    digitalWrite(pinC, LOW);

    pinMode(pinD, OUTPUT);
    digitalWrite(pinD, LOW);

    Serial.begin(115200);
    inputString.reserve(64);   // 64 bytes should be sufficient!
    //cmdstr.reserve(64);   // 64 bytes should be sufficient!
    //cmdpara.reserve(64);   // 64 bytes should be sufficient!

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
        sei();    // enable interrups
        Serial.println("OK - Pulse(s) done!");     
        
        // reset run mode to wait for new command!
        runmode = "";
        
    }   // end "pulse"
      
    else if (runmode == "periodic"){     // periodic pulses until new command
          // parameters have been parsed already in SerialEvent
          // here, just send a single pulse, then in next iteration of main loop
          // this is for sending short pulses (strict timing) in less strict periods (milliseconds to seconds)
          cli();    // disable interrupts
          PIND=(1<<pinID);
          delayMicroseconds(pulselen+delta_t);
          PIND=(1<<pinID);
          sei();    // enable interrups

          delayMicroseconds(pulsegap);   // spacing between pulses    
    }   // end "periodic "

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
        sei();    // enable interrups
        delay(1);   // 1 ms, resulting in approx 1000 Hz
    }   // end "test "


    // else pass;


}   // end main loop     
  

 

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.

  Commands terminate with "\n", run mode is parsed from first word,
  other parameters as neeeded, all separated by space.

  rsxy: borrowed from some example (serial string parsing?), 
        extended for immediate parsing and setting run mode.
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
      else if (inputString.startsWith("test")){  
          Serial.println("OK - Test mode, sending pulses of varying length!");     
          runmode = "test";
      }
      else if (inputString.startsWith("help")){
            
          Serial.println(helpstr);     
          runmode = "";  // empty string, so no specific mode will be entered
      }
      else if (inputString.startsWith("stop")){  
          Serial.println("OK - Stop mode, waiting for new command..");     
          runmode = "";  // empty string, so no specific mode will be entered
      }
       
      else{
        Serial.println("Warning: Command could not be parsed! Try 'test' for 1, 2, 5, 10 µs test pulse");
        // don't change run mode, just go on with whatever is was
        // runmode = "";
      }
          
      inputString = "";
     
      stringComplete = true;
    }
  }
}
