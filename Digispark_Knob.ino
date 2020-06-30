/*
 * PINOUT, Digispark to KY-040
 * P2   to  CLK
 * P1   to  DT
 * P0   to  SW
 * VIN  to  GND
 * GND  to  +5V
 */

//Pin settings
#define encoderPinA 2 //clk
#define encoderPinB 1 //dt
#define encoderButton 0 //sw

#define LEFT_ACTION 0x50
#define LEFT_ACTION_MODE1 0x50

#define RIGHT_ACTION 0x4f
#define RIGHT_ACTION_MODE1 0x4f

#define BUTTON_ACTION_SINGLE 0x0

#define MMKEY_VOL_UP       175
#define MMKEY_VOL_DOWN      174
#define MMKEY_PLAYPAUSE      0xCD

// Debug option, uncomment next line if you want to output commands as text (e.g. to Notepad).
//#define KB_DEBUG

#include <DigiKeyboard.h>

#define LATCHSTATE 3
int buttonState = LOW, lastButtonState = LOW;
long lastDebounceTime = 0, debounceDelay = 50;
int _position = 0, _positionExt = 0;
int8_t _oldState; bool btnPressed=false, btnReleased=false, pressedRotary=false;
boolean first = true;
int mode = 1;

const int8_t encoderStates[] = {
  0, -1, 1, 0,
  1, 0, 0, -1,
  -1, 0, 0, 1,
  0, 1, -1, 0
};

void setup() {
  
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  pinMode(encoderButton, INPUT);
  digitalWrite(encoderPinA, HIGH);
  digitalWrite(encoderPinA, HIGH);
  digitalWrite(encoderButton, LOW);
  _oldState = 3; 
}

void loop() {
  static int pos = 0;
  tick();
  int newPos = getPosition();
  if (pos != newPos) {
    #ifndef KB_DEBUG
    if (first == true && mode == 1 && !btnPressed) {
      DigiKeyboard.sendKeyPress(0x2b, MOD_ALT_LEFT);
      DigiKeyboard.sendKeyPress(0x0, MOD_ALT_LEFT);
    }
    #endif
    if (newPos < pos) {
      if (!btnPressed) {
        #ifndef KB_DEBUG
        if (mode == 1){
          DigiKeyboard.sendKeyPress(LEFT_ACTION, MOD_ALT_LEFT);
          DigiKeyboard.sendKeyPress(0x0, MOD_ALT_LEFT);
        } else if (mode == 2){
          DigiKeyboard.sendKeyStroke(0x2b, MOD_CONTROL_LEFT | MOD_SHIFT_LEFT);
        }
        #else
          DigiKeyboard.println("LEFT_ACTION");
        #endif
      } else {
        pressedRotary=true;
        #ifndef KB_DEBUG
          mode--;
          if (mode < 1) mode = 1;
          DigiKeyboard.sendKeyStroke(BUTTON_ACTION_SINGLE);
        #else
          DigiKeyboard.println("LEFT_ACTION_MODE1");
          mode--;
          if (mode < 1) mode = 1;
        #endif
      }
      first = false;
    }
    else if (newPos > pos){
     if (!btnPressed) {
        #ifndef KB_DEBUG
        if (mode == 1){
          DigiKeyboard.sendKeyPress(RIGHT_ACTION, MOD_ALT_LEFT);
          DigiKeyboard.sendKeyPress(0x0, MOD_ALT_LEFT);
        } else if (mode == 2){
          DigiKeyboard.sendKeyStroke(0x2b, MOD_CONTROL_LEFT);
        }
        #else
          DigiKeyboard.println("RIGHT_ACTION");
        #endif
      } else {
        pressedRotary=true;
        #ifndef KB_DEBUG
          mode++;
          if (mode > 2) mode = 2;
          DigiKeyboard.sendKeyStroke(0);
        #else
          DigiKeyboard.println("RIGHT_ACTION_MODE1");
          mode++;
          if (mode > 2) mode = 2;
        #endif
      }
      first = false;
    }
    pos = newPos;
  }
  int reading = digitalRead(encoderButton);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        btnPressed=true;
      } else if (buttonState == LOW){
        btnReleased=true;
      }
    } 
  }
  lastButtonState = reading;

  if (btnPressed == true && btnReleased ==true && pressedRotary ==false) {
    #ifndef KB_DEBUG
      if (mode == 1){
          DigiKeyboard.sendKeyStroke(BUTTON_ACTION_SINGLE);
        } else if (mode == 2){
          DigiKeyboard.write(' ');
        }
      first = true;
    #else
      DigiKeyboard.println(btnPressed);
      DigiKeyboard.println(btnReleased);
      DigiKeyboard.println(pressedRotary);
      DigiKeyboard.println(mode);
      DigiKeyboard.println("BUTTON_ACTION_SINGLE");
    #endif
    btnPressed=false;btnReleased=false;
  } else if (btnPressed == true && btnReleased == true && pressedRotary == true) {
    #ifdef KB_DEBUG
      DigiKeyboard.println("RELEASED_AFTER_HELD_ROTATION");
    #endif
    btnPressed=false;btnReleased=false;pressedRotary=false;
  }
  DigiKeyboard.update();
}

int  getPosition() {
  return _positionExt;
}

void setPosition(int newPosition) {
  _position = ((newPosition<<2) | (_position & 0x03));
  _positionExt = newPosition;
}

void tick(void) {
  int sig1 = digitalRead(encoderPinA);
  int sig2 = digitalRead(encoderPinB);
  int8_t thisState = sig1 | (sig2 << 1);
  if (_oldState != thisState) {
    _position += encoderStates[thisState | (_oldState<<2)];
    if (thisState == LATCHSTATE)
      _positionExt = _position >> 2;
    _oldState = thisState;
  }
} 
