#include <TimerOne.h>
#include <Encoder.h>

//CHANNEL
int channelNumber = 1; //each module should have a unique channel number between 1 and 125 (126 & 127 are used by the encoders).
int pbBounce = 150; //150 millisecond debounce duration - you may want to change this value depending on your pushbuttons

//PUSHBUTTON
// '1' == Button verbunden
// '0' == Button verbunden
// '9' == Encoder verbunden
int toReadPushButton[38] =
{ //Pin number are written below
  9, 9, 9, 9, 1, //0-4
  1, 9, 1, 1, 1, //5-9
  1, 1, 1, 1, 0, //10-14
  0, 0, 0, 9, 9, //15-19
  1, 1, 1, 1, 1, //20-24
  0, 0, 0, 0, 0, //25-29
  0, 0, 0, 0, 0, //30-34
  0, 9, 9     //35-37
};
//pushbutton mode
//there are a few different modes in which you may wish for your pushbutton to behave
//'1' - standard mode - when pushbutton is engaged note is turned on, when pushbutton is released, note is turned off
//'2' - on mode - note is only turned on with each click
//'3' - off mode - note is only turned off with each click
//'4' - toggle mode - note is switched between on and off with each click
//pins with '9' are those which are encoders and should not be used as pushbuttons unless necessary
int pushbuttonMode[76] =
{ //Pin number are written below
  9, 9, 9, 9, 1, //0-4
  1, 9, 1, 1, 1, //5-9
  1, 1, 1, 1, 0, //10-14
  0, 0, 0, 9, 9, //15-19
  1, 1, 1, 1, 1, //20-24
  0, 0, 0, 0, 0, //25-29
  0, 0, 0, 0, 0, //30-34
  0, 9, 9,    //35-37
  9, 9, 9, 9, 0, //38-42 SHIFT
  0, 9, 0, 0, 0, //43-47 SHIFT
  0, 0, 0, 0, 0, //48-52 SHIFT
  0, 0, 0, 9, 9, //53-57 SHIFT
  0, 0, 0, 0, 0, //58-62 SHIFT
  0, 0, 0, 0, 0, //63-67 SHIFT
  0, 0, 0, 0, 0, //68-72 SHIFT
  0, 9, 9     //73-75 SHIFT
};

//SHIFT
int shiftPin = 0; //if using a shift button enter the pin number here, else put 0

//ANALOG IN MUX
//CD4067BE - http://www.ti.com/lit/ds/symlink/cd4067b.pdf
//'1' for multiplexer inputs you want to read, else enter '0'
int toReadMux[16] =
{ //IC pin number are written below
  1, 1, 1, 1, //4-7
  1, 1, 1, 1,
  2, 2, 2, 2,
  2, 2, 2, 2
};

int MUXupdateCounter = 15;



// VARIABLES AND FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//PUSHBUTTONS
long timeHit[76]; //38*2 = 76 (shift button)
boolean buttonState[76 + 8]; //stored state: if the button was last turned on or off
int shiftChange;
//ENCODER
int encoderPins[4][2] = {{0, 1}, {2, 3}, {36, 37}, {18, 19}}; //encoder pin numbers
Encoder enc1(0, 1);
Encoder enc2(2, 3);
Encoder enc3(18, 19);

//ANALOG IN
int s0 = 14; //control pin A
int s1 = 15; //control pin B
int s2 = 16; //control pin C
int s3 = 17; //control pin D
int SIG_pin = 43; //read pin
int analogInsPrev[48]; //array to hold previously read analog values - set all to zero for now
int tempAnalogIn = 0; //array to hold previously read analog values
int tempAnalogInMap = 0;
int analogThreshold = 4; //threshold
int controlPin[] = {s0, s1, s2, s3}; //set contol pins in array
//control array
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

//LED-Status
bool ledStatus[2];
bool syncStatus[2];
bool blinkMode[2];
bool loopOn[2];

int readPoti(int num) {
  int out = 0;
  for (int i=0;i<3;i++) {
    out += analogRead(num);
    delayMicroseconds(30);
  }
  return (out/3);
}


//function to read mux
int readMux(int channel, int no) {

  //loop through the four control pins
  for (int i = 0; i < 4; i ++) {
    //turn on/off the appropriate control pins according to what channel we are trying to read
    digitalWrite(controlPin[i], muxChannel[channel][i]);
    //digitalWrite(controlPin[i], bitRead(channel,3-i));
  }

  int val;

  //read the value of the pin
  if ((no == 2 && ( (channel < 4) || (channel > 7 && channel < 12) )) || (no == 0 && !(channel < 12 && channel > 7))) {
    pinMode(SIG_pin + no, INPUT_PULLUP);
    delay(1);
    val = digitalRead(SIG_pin + no);
    pinMode(SIG_pin + no, INPUT);
  }
  else val = readPoti(SIG_pin + no);
  //return the value
  return val;
}
int analogInsPrevTeensy[8]; //array to hold previously read analog values
int tempAnalogInTeensy = 0;
int tempAnalogInMapTeensy = 0;
int analogThresholdTeensy = 4; //threshold



// SETUP (pin config) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
void setup() {

  Serial.begin(31250);//open serail port @ midi speed

  //PUSHBUTTON pin config
  //we need enable each pushbutton pin as an INPUT as well as turn on the pullup resistor
  for (int i = 0; i < 38; i++) {
    if (toReadPushButton[i] == 1) {
      pinMode(i, INPUT_PULLUP); //pushbutton pullup
    }
  }

  //LED pin config
  //we need enable each LED pin as an OUTPUT
  Timer1.initialize(5000);
  Timer1.attachInterrupt(blinkLED);

  usbMIDI.setHandleTimeCodeQuarterFrame(syncSig);
  usbMIDI.setHandleNoteOn(OnNoteOn) ;
  usbMIDI.setHandleNoteOff(OnNoteOff) ;

  enc1.write(64); enc2.write(64); enc3.write(64);

  //SHIFT pin config
  //we need enable the shift pin as an INPUT as well as turn on the pullup resistor
  if (shiftPin != 0) {
    pinMode(shiftPin, INPUT_PULLUP); //shift button
  }

  //ANALOG IN MUX pin config
  //set analog in reading
  pinMode(SIG_pin, INPUT);
  //set our control pins to output
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  //turn all control pins off (for now)
  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

}



// LOOPS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
void loop() {

  // Loop through the Buttons
  boolean tempDigitalRead;
  for (int i = 0; i < 38; i++) { //loop through all 38 digital pins
    int j = i + shiftChange; //add the shift change    TODO remove shift change
    if (toReadPushButton[i] == 1) { //check if this a pin with a pushbutton hooked up to it
      tempDigitalRead = digitalRead(i);
      if (pushbuttonMode[j] == 1) { //normal mode
        if (tempDigitalRead != buttonState[j]) { //something has changed
          delay(2); //just a delay for noise to ensure push button was actually hit
          if (digitalRead(i) == tempDigitalRead) { //check if pushbutton is still the same
            if (tempDigitalRead == LOW) { //button pressed, turn note on
              midiNoteOnOff(1, j); //call note on/off function
            }
            else { //button released
              midiNoteOnOff(0, j);
            }
            buttonState[j] = tempDigitalRead; //update the state (on or off)
          }
        }
      }
      else { //all other modes (2,3,4)
        if (digitalRead(i) == LOW) { //button was pressed
          if ((millis() - timeHit[j]) > pbBounce) { //check bounce time
            if (pushbuttonMode[j] == 2) { //mode 2 - only note on
              midiNoteOnOff(1, j);
            }
            else if (pushbuttonMode[j] == 3) { //mode 3 - only note off
              midiNoteOnOff(0, j);
            }
            else { //mode 4 - toggle
              if (buttonState[j] == 1) {
                midiNoteOnOff(0, j);
                buttonState[j] = 0;
              }
              else {
                midiNoteOnOff(1, j);
                buttonState[j] = 1;
              }
            }
            timeHit[j] = millis();
          }
        }
      }
    }
  }

  // 3 Encoders: Jog wheels & Menu selector
  
  int encVal[] = {enc1.read(), enc2.read(), enc3.read()};
  
  for (int i = 0; i < 3; i++) {
    // Act on the encoder value
    if (encVal[i] != 64) {
      //usbMIDI.sendNoteOn(i, encVal[i], 1);
      usbMIDI.sendControlChange(240+i, encVal[i], channelNumber);
    }
  }
  // Reset controllers to normal position
  enc1.write(64); enc2.write(64); enc3.write(64);

  // Loop through all the multiplexers
  for (int i = 0; i < 16; i++) {
    if (!((i < 4) || (i > 7 && i < 12))) { //check if this a pin with a analog input hooked up to it
      tempAnalogIn = readMux(i, 2); //ready valued using readMux function
      if (abs(analogInsPrev[i] - tempAnalogIn) > analogThreshold) { //ensure value changed more than our threshold
        if (i!=0&&i!=2||abs(analogInsPrev[i] - tempAnalogIn) > analogThreshold*3) {
          tempAnalogInMap = map(tempAnalogIn, 0, 1023, 0, 127); //remap value between 0 and 127
          usbMIDI.sendControlChange(i, tempAnalogInMap, channelNumber);
          analogInsPrev[i] = tempAnalogIn; //reset current value
        }
      }
    } else {
      bool tempDigitalRead = readMux(i, 2);
      if (tempDigitalRead != analogInsPrev[i]) { //something has changed
        delay(2); //just a delay for noise to ensure push button was actually hit
        if (readMux(i, 2) == tempDigitalRead) { //check if pushbutton is still the same
          if (tempDigitalRead == LOW) { //button pressed, turn note on
            midiNoteOnOff(1, i + 76); //call note on/off function
          } else { //button released
            midiNoteOnOff(0, i + 76);
          }
          analogInsPrev[i] = tempDigitalRead; //update the state (on or off)
        }
      }
    }
  }

  for (int i = 0; i < 16; i++) {
    tempAnalogIn = readMux(i, 1); //ready valued using readMux function
    if (abs(analogInsPrev[i + 16] - tempAnalogIn) > analogThreshold) { //ensure value changed more than our threshold
      tempAnalogInMap = map(tempAnalogIn, 0, 1023, 0, 127); //remap value between 0 and 127
      usbMIDI.sendControlChange(i + 16, tempAnalogInMap, channelNumber);
      analogInsPrev[i + 16] = tempAnalogIn; //reset current value
    }
  }

  for (int i = 0; i < 16; i++) {
    tempAnalogIn = readMux(i, 0); //ready valued using readMux function
    if (i < 12 && i > 7) {
      if (abs(analogInsPrev[i + 32] - tempAnalogIn) > analogThreshold) { //ensure value changed more than our threshold
        tempAnalogInMap = map(tempAnalogIn, 0, 1023, 0, 127); //remap value between 0 and 127
        usbMIDI.sendControlChange(i + 32, tempAnalogInMap, channelNumber);
        analogInsPrev[i + 32] = tempAnalogIn; //reset current value
      }
    } else {
      bool tempDigitalRead = readMux(i, 0);
      if (tempDigitalRead != analogInsPrev[i + 32]) { //something has changed
        delay(2); //just a delay for noise to ensure push button was actually hit
        if (readMux(i, 0) == tempDigitalRead) { //check if pushbutton is still the same
          if (tempDigitalRead == LOW) { //button pressed, turn note on
            midiNoteOnOff(1, i + 92); //call note on/off function
          } else { //button released
            midiNoteOnOff(0, i + 92);
          }
          analogInsPrev[i + 32] = tempDigitalRead; //update the state (on or off)
        }
      }
    }
  }

  //Loop through the Teensy analog channels
  for (int i = 0; i < 4; i++) { //loop through the 8 analog teensy channels
    tempAnalogInTeensy = readPoti(i + 39);
    delay(2);
    tempAnalogInTeensy = (tempAnalogInTeensy + readPoti(i + 39))/2;
    if (abs(analogInsPrevTeensy[i] - tempAnalogInTeensy) > 1) {
    if (i==0||i==2) {
        if (loopOn[i/2]) {
          tempAnalogInMapTeensy = map(tempAnalogInTeensy, 0, 1023, 0, 127);
          usbMIDI.sendControlChange(i, tempAnalogInMapTeensy, channelNumber);
        }
    } else {
        usbMIDI.sendControlChange(i, map(tempAnalogInTeensy, 0, 1023, 0, 127), channelNumber);
        if (((i==1&&syncStatus[0]) || (i==3&&syncStatus[1])) ) {
          tempAnalogInMapTeensy = map(tempAnalogInTeensy, 0, 1023, -8192, 8191);
          usbMIDI.sendPitchBend(tempAnalogInMapTeensy, 3+(i-1)/2);
        }
      }
      analogInsPrevTeensy[i] = tempAnalogInTeensy;
    }
  }
}


// INIT all the analog values in case the controller has different values than the Program
void initMUXvalues() {

  //ANALOG IN MUX loops
  for (int i = 0; i < 16; i++) {
    if (!((i < 4) || (i > 7 && i < 12))) { //check if this a pin with a analog input hooked up to it
      tempAnalogIn = readMux(i, 2); //ready valued using readMux function
      tempAnalogInMap = map(tempAnalogIn, 0, 1023, 0, 127); //remap value between 0 and 127
      usbMIDI.sendControlChange(i, tempAnalogInMap, channelNumber);
    }
  }

  for (int i = 0; i < 16; i++) {
    tempAnalogIn = readMux(i, 1); //ready valued using readMux function
    tempAnalogInMap = map(tempAnalogIn, 0, 1023, 0, 127); //remap value between 0 and 127
    usbMIDI.sendControlChange(i + 16, tempAnalogInMap, channelNumber);
  }


  for (int i = 0; i < 16; i++) {
    tempAnalogIn = readMux(i, 0); //ready valued using readMux function
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


// LED Plink der Play Taste
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

//function to send note on/off
//this helps to keep the code concise
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
