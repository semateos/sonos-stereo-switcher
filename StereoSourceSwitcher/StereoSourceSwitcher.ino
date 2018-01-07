/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

int RECV_PIN = 11;
int BUTTON_PIN = 12;
int STATUS_PIN = 13;

IRsend irsend;
IRrecv irrecv(RECV_PIN);

unsigned long CD_MODE = 0x4BB6906F;
unsigned long V2_MODE = 0x4BB6708F;

decode_results results;
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
int lastButtonState;
boolean sending = false;

void setup()
{

  irrecv.enableIRIn(); // Start the receiver
  pinMode(BUTTON_PIN, INPUT);
  pinMode(STATUS_PIN, OUTPUT);

  // initialize serial:
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Serial Connected!");
  
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}


void repeatIR(unsigned long hexCommand) {

  //sending = true;
  
  for (int i = 0; i < 4; i++) {
    
    irsend.sendNEC(hexCommand, 32);
    delay(100); // Wait a bit between retransmissions
  }
  
  irrecv.enableIRIn();
}

// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state

// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results *results) {

  //Serial.println("Decoding IR input");
  
  codeType = results->decode_type;
  //int count = results->rawlen;
  if (codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= codeLen; i++) {
      if (i % 2) {
        // Mark
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        // Space
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(rawCodes[i - 1], DEC);
    }
    Serial.println("");
  }
  else {
    if (codeType == NEC) {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (codeType == SONY) {
      Serial.print("Received SONY: ");

      //if waking sony tv, switch stereo to video 2:
      if(results->value == 0x750){
        
        //Serial.print("HEREEREERE");
        repeatIR(V2_MODE);
      }
      
    } 
    else if (codeType == PANASONIC) {
      Serial.print("Received PANASONIC: ");
    }
    else if (codeType == JVC) {
      Serial.print("Received JVC: ");
    }
    else if (codeType == RC5) {
      Serial.print("Received RC5: ");
    } 
    else if (codeType == RC6) {
      Serial.print("Received RC6: ");
    } 
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(codeType, DEC);
      Serial.println("");
    }
    Serial.println(results->value, HEX);
    codeValue = results->value;
    codeLen = results->bits;
    //Serial.println(codeLen);
  }
}

void loop() {
	
	// If button pressed, send the code.
  int buttonState = digitalRead(BUTTON_PIN);
  
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("Released");
  }

  if (buttonState) {
    Serial.println("Pressed, sending");
    digitalWrite(STATUS_PIN, HIGH);

    repeatIR(V2_MODE);
    
    //irsend.sendSony(0x750, 12);
    
    digitalWrite(STATUS_PIN, LOW);
    delay(50); // Wait a bit between retransmissions
  } 

  lastButtonState = buttonState;

  //Serial.println("Checking IR in");
  //watch for IR in
  if (irrecv.decode(&results)) {
    Serial.println("IR decode results");
    digitalWrite(STATUS_PIN, HIGH);
    storeCode(&results);
    irrecv.resume(); // resume receiver
    digitalWrite(STATUS_PIN, LOW);
  }
    
  // print the string when a newline arrives:
  if (stringComplete) {
    
    Serial.print("Received: " + inputString);

    if(inputString == "CD\n"){

      repeatIR(CD_MODE);
      
      Serial.print("Switched to: " + inputString);
    }

    if(inputString == "V2\n"){

      repeatIR(V2_MODE);
      
      Serial.print("Switched to: " + inputString);
    }
    
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}

void serialEvent() {
  while(Serial.peek() > 0) {
  //while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      return;
    }
  }
}
