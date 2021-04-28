#include <TimerOne.h>
#include <Encoder.h>

//Channel number for the TeensyMIDI
#define channelNumber 1

//Time to debounce the buttons in ms
#define pbBounce 150

// MUX STUFF 
#define SIG_pin 43 //read pin
#define analogThreshold 5 //threshold



// The status of the three shift buttons (left/right deck & track select)
bool shiftKey[3] = {false, false, false};
char shiftPins[3][2] = {{1,2},{0,10},{0,4}};
char shiftOld[3] = {0,0,0};

int MUXupdateCounter = 15;

//ENCODERS
Encoder enc1(18, 19);
Encoder enc2(2, 3);
Encoder enc3(0, 1);

int controlPin[] = {14, 15, 16, 17}; //set contol pins in array

//control array for the mux pins, TODO remove this
int muxChannel[16][4] = {
  {0, 0, 0, 0}, //channel 0
  {1, 0, 0, 0}, //channel 1
  {0, 1, 0, 0}, //channel 2
  {1, 1, 0, 0}, //channel 3
  {0, 0, 1, 0}, //channel 4
  {1, 0, 1, 0}, //channel 5
  {0, 1, 1, 0}, //channel 6
  {1, 1, 1, 0}, //channel 7
  {0, 0, 0, 1}, //channel 8
  {1, 0, 0, 1}, //channel 9
  {0, 1, 0, 1}, //channel 10
  {1, 1, 0, 1}, //channel 11
  {0, 0, 1, 1}, //channel 12
  {1, 0, 1, 1}, //channel 13
  {0, 1, 1, 1}, //channel 14
  {1, 1, 1, 1} //channel 15
};

//LED-Status blink
bool ledStatus[2];
bool syncStatus[2];
bool blinkMode[2];
bool loopOn[2];



// CODE BEGIN  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //


// Read an analog value from a Teensy analog pin
int readPoti(int num) {
  int out = 0;
  // Do 3 measurements and return the mean value
  for (int i=0;i<3;i++) {
    out += analogRead(num);
    delayMicroseconds(30);
  }
  return out/3;
}

bool readBtn(int num) {
  return digitalRead(num);
}


//Read a Value from the MUX
int readMux(int channel, int no, bool analog) {

  // Set the four MUX control pins
  for (int i = 0; i < 4; i ++) digitalWrite(controlPin[i], muxChannel[no][i]);

  if (analog) { // Read a analog value from a MUX
    pinMode(SIG_pin + channel, INPUT);
    delayMicroseconds(50);
    return readPoti(SIG_pin + channel);
  } else { // Read a digital button value from a MUX
    pinMode(SIG_pin + channel, INPUT_PULLUP);
    delayMicroseconds(50);
    bool btnVal = readBtn(SIG_pin + channel);
    return btnVal;
  }
}


// Reads a value from a MUX or directly from the Teensy
int readValue(int channel, int no, bool analog) {
  if (channel == 0) {
    if (analog) return readPoti(no);
    else return readBtn(no);
  } else {
    return readMux(channel - 1, no, analog);
  }
}



// All the pins, their function and their MIDI no [MIDIno, MUXno, PINno] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

// Analog values
char anaConf[32][3] =
{
  // THE LOW MID HIGH MIXER VALUES
  {0x01, 2, 12},
  {0x02, 2, 13},
  {0x03, 2, 14},
  
  {0x04, 2,  0},
  {0x05, 2,  1},
  {0x06, 2,  2},
  
  {0x07, 2,  8},
  {0x08, 2,  9},
  {0x09, 2, 10},
  
  {0x0A, 2,  4},
  {0x0B, 2,  5},
  {0x0C, 2,  6},
  
  // THE VOLUME VALUES
  {0x0D, 1,  8},
  {0x0E, 1, 11},
  {0x0F, 1,  9},
  {0x10, 1, 10},
  
  // HIGH PASS FILTERS
  {0x11, 2, 15},
  {0x12, 2,  3},
  {0x13, 2, 11},
  {0x14, 2,  7},
  
  // FX 1
  {0x15, 3,  7},
  {0x16, 3,  5},
  {0x17, 3,  6},
  {0x18, 3,  4},
  
  // FX 2
  {0x19, 3, 12},
  {0x1A, 3, 13},
  {0x1B, 3, 14},
  {0x1C, 3, 15},

  // ~~~~~~~ THE FOLLOWING PART DEPENDS ON SHIFT KEYS ~~~~~~~~

  // DECKS speed, loop size
  {0x20, 0, 42},
  {0x22, 0, 41},

  {0x26, 0, 40},
  {0x28, 0, 39},
};

// Digital values (buttons)
char btnConf[31][3] =
{
  // Headphone on
  {0x08, 1,12},
  {0x09, 1,13},
  {0x0A, 1,14},
  {0x0B, 1,15},
  
  // FX SELECT
  {0x10, 0,24},
  {0x11, 0,23},
  {0x12, 0,22},
  {0x13, 0,21},
  {0x14, 0,21},

  // FX1
  {0x15, 3, 3},
  {0x16, 3, 2},
  {0x17, 3, 1},
  {0x18, 3, 0},

  // FX2
  {0x19, 3, 8},
  {0x1A, 3, 9},
  {0x1B, 3, 10},
  {0x1C, 3, 11},


  // DECK CONTROL LEFT
  {0x20, 1, 0}, //PLAY
  {0x21, 1, 1}, //CUE
  {0x22, 1, 3}, //SYNC
  {0x23, 1, 4}, //LOOP1
  {0x24, 1, 5}, //LOOP2
  {0x25, 1, 6}, //MOVE1
  {0x26, 1, 7}, //MOVE2

   // DECK CONTROL RIGHT
  {0x30, 0, 12}, //PLAY
  {0x31, 0, 11}, //CUE
  {0x32, 0, 9}, //SYNC
  {0x33, 0, 8}, //LOOP1
  {0x34, 0, 7}, //LOOP2
  {0x35, 0, 6}, //MOVE1 // TODO ?
  {0x36, 0, 5}, //MOVE2
};

// previous values
int anaOld[32] = {};
int digOld[31] = {};


// SETUP (pin config) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
void setup() {

  Serial.begin(31250);

  for (int i=0; i < 31; i++) {
    if (btnConf[i][1] == 0) { // Only set Pins for Teensy Analog Inputs
      pinMode(btnConf[i][2], INPUT_PULLUP);
    }
  }
  //Set Shift Pins to Pullup
  pinMode(4, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);

  //LED pin config
  //we need enable each LED pin as an OUTPUT
  Timer1.initialize(5000);
  Timer1.attachInterrupt(blinkLED);

  usbMIDI.setHandleTimeCodeQuarterFrame(syncSig);
  usbMIDI.setHandleNoteOn(OnNoteOn) ;
  usbMIDI.setHandleNoteOff(OnNoteOff) ;

  // Reset the encoder values
  enc1.write(64); enc2.write(64); enc3.write(64);

  // ANALOG IN MUX pin config
  //set analog in reading
  pinMode(SIG_pin, INPUT);
  //set our control pins to low output
  for (char i = 0; i < 4; i++) {
     pinMode(controlPin[i], OUTPUT);
     digitalWrite(controlPin[i], LOW);
  }
}


// LOOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
void loop() {

  // 3 Encoders: Jog wheels & Menu selector
  int encVal[] = {enc1.read(), enc2.read(), enc3.read()};
  for (int i = 0; i < 3; i++) {
    // Act on the encoder value
    if (encVal[i] != 64) {
      int ccNo = 0x21+4*i;
      // Apply shift for scratching
      if ( (ccNo == 0x21 && shiftKey[0]) ||  (ccNo == 0x29 && shiftKey[1]) ) ccNo += 0x10; //+16 
      
      usbMIDI.sendControlChange(ccNo, encVal[i], channelNumber);
    }
  }
  // Reset controllers to normal position
  enc1.write(64); enc2.write(64); enc3.write(64);

  // Loop through analog values
  for (char i = 0; i < 32; i++) {
    int readVal = readValue(anaConf[i][1], anaConf[i][2], true);
    if (abs(anaOld[i]-readVal) > analogThreshold) {
      char ccNo = anaConf[i][0];

      // Apply the shift key for the Speed and Loopsize
      if (ccNo > 0x1F) {
         if (ccNo < 0x25 && shiftKey[0]) ccNo += 8;
         else if (ccNo > 0x25 && shiftKey[1]) ccNo += 8;
      }
      
      usbMIDI.sendControlChange(ccNo, map(readVal, 0, 1023, 0, 127), channelNumber);
      anaOld[i] = readVal;
    }
  }
  
  // Loop through digital values
  for (char i = 0; i < 31; i++) {
    bool readVal = readValue(btnConf[i][1], btnConf[i][2], false);
    if (readVal != digOld[i]) {
      int ccNo = btnConf[i][0];

      // Shift for Headphone Btns
      if (ccNo < 0x10) {
        if (shiftKey[2]) ccNo += 0x04;
      }
      //Shift for Decks
      else if (ccNo > 0x1F) {
         if (ccNo < 0x30 && shiftKey[0]) ccNo += 32;
         else if (ccNo > 0x2F && shiftKey[1]) ccNo += 32;
      }
      
      midiNoteOnOff(!readVal, ccNo);
      digOld[i] = readVal;
    }
  }


  // Implement the shift keys
  for (char i = 0; i < 3; i++) {
    bool val = !readValue(shiftPins[i][0], shiftPins[i][1], false);
    if (val && !shiftOld[i]) {
      shiftKey[i] = !shiftKey[i];
    }
    shiftOld[i] = val;
  }

  delay(30);

}












// INIT all the analog values in case the controller has different values than the Program
void initMUXvalues() {

  int tempAnalogInTeensy = 0;
  int tempAnalogInMapTeensy = 0;
  int tempAnalogIn = 0;
  int tempAnalogInMap = 0;

  //ANALOG IN MUX loops
  for (int i = 0; i < 16; i++) {
    if (!((i < 4) || (i > 7 && i < 12))) { //check if this a pin with a analog input hooked up to it
      tempAnalogIn = readMux(i, 2, true); //ready valued using readMux function
      tempAnalogInMap = map(tempAnalogIn, 0, 1023, 0, 127); //remap value between 0 and 127
      usbMIDI.sendControlChange(i, tempAnalogInMap, channelNumber);
    }
  }

  for (int i = 0; i < 16; i++) {
    tempAnalogIn = readMux(i, 1, true); //ready valued using readMux function
    tempAnalogInMap = map(tempAnalogIn, 0, 1023, 0, 127); //remap value between 0 and 127
    usbMIDI.sendControlChange(i + 16, tempAnalogInMap, channelNumber);
  }


  for (int i = 0; i < 16; i++) {
    tempAnalogIn = readMux(i, 0, true); //ready valued using readMux function
    if (i < 12 && i > 7) {
      tempAnalogInMap = map(tempAnalogIn, 0, 1023, 0, 127); //remap value between 0 and 127
      usbMIDI.sendControlChange(i + 32, tempAnalogInMap, channelNumber);
    }
  }

  //ANALOG IN TEENSY loop
  for (int i = 1; i < 4; i++) { //loop through the 8 analog teensy channels
    if (i != 2) {
      tempAnalogInTeensy = readPoti(i + 39);
      tempAnalogInMapTeensy = map(tempAnalogInTeensy, 0, 1023, -8192, 8191);
      if (((i==1&&syncStatus[0]) || (i==3&&syncStatus[1])) ) {
        usbMIDI.sendPitchBend(tempAnalogInMapTeensy, 3+(i-1)/2);
      }
    }
  }

  usbMIDI.sendControlChange(48, 127, channelNumber);
}




// LED Blink der Play Taste
byte blinkLEDcounter = 0;

void blinkLED() {
  usbMIDI.read();
  if (blinkLEDcounter == 0) {
    if (!(ledStatus[0]||ledStatus[1])) {
    digitalWrite(25,!digitalRead(25));
    digitalWrite(26,!digitalRead(25));
    } else {
      if (!ledStatus[0]) digitalWrite(25,!digitalRead(25));
      if (!ledStatus[1]) digitalWrite(26,!digitalRead(26));
    }
  }
  if (blinkLEDcounter == 0) {
    MUXupdateCounter++;
    if (MUXupdateCounter == 20) {
      //initMUXvalues();
      MUXupdateCounter = 0;
    }
  }
  blinkLEDcounter = (blinkLEDcounter + 1) % 100;
}


// COMMUNICATION FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void midiNoteOnOff(boolean onoff, int q) {
  if (onoff == 1) { //note on
    usbMIDI.sendNoteOn(q, 127, channelNumber);
  }
  else { //note off
    usbMIDI.sendNoteOff(q, 127, channelNumber);
  }
}

void OnNoteOn(byte channel, byte note, byte velocity)
{
  if (note<2) {
    ledStatus[note] = 1;
    digitalWrite(note + 25, blinkMode[note]);
    return;
  } 
  if (note < 4) {
    syncStatus[note-2] = 1;
    return;
  }
  if (note < 6) {
    blinkMode[note-4] = 0;
    if (ledStatus[note-4]) digitalWrite(note + 21, 0);
    return;
  }
  if (note < 8) {
    loopOn[note-6] = 1;
    return;
  }
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
  if (note<2) {
    ledStatus[note] = 0;
    return;
  } 
  if (note < 4) {
    syncStatus[note-2] = 0;
    return;
  }
  if (note < 6) {
    blinkMode[note-4] = 1;
    if (ledStatus[note-4]) digitalWrite(note + 21, 1);
    return;
  }
  if (note < 8) {
    loopOn[note-6] = 0;
    return;
  }
}

void syncSig(byte b) {
  
}
