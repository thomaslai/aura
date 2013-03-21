//#include <SoftwareSerial.h>
#include <string.h>
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

///////////////////////////////////////////////////////////////////////////////
// PEENS  PEENS  PEENS  PEENS  PEENS  PEENS  PEENS  PEENS  PEENS  PEENS  PEENS/
///////////////////////////////////////////////////////////////////////////////
//Non coding pins:
//  RF Module:
//    UNUSABLE = 10
//    MOSI = 11
//    MISO = 12
//    SCK = 13

// LED pins
const int LEFT_LED = 2;     // the number of the LED pin
const int RIGHT_LED = 8;
const int BRAKE_LED = 5;		// 2 and 8 aren't PWM
const int ON_BOARD_LED = 13;

// RF pins
const int CE_PIN = 9; // To be changed to A0
const int CSN_PIN = 10; // To be changed to A1

// Battery indicator
const int BATTERY_PIN = A7;
const int BATTERY_LED = 3; // After we transitioned to the matrix

////////////////////////////////////////////////////////////////////////////////
// CONSTANTS  CONSTANTS  CONSTANTS  CONSTANTS  CONSTANTS  CONSTANTS  CONSTANTS//
////////////////////////////////////////////////////////////////////////////////
// Serial setup
//SoftwareSerial Soft_Serial(10, 11); // RX, TX
const int BAUDRATE = 9600;
const int DELAYVAL = 100;
const int FEEDBACK_CHAR_LENGTH = 4;

// Soft Serial Char Mapping
const char LEFT_RISE_CHAR = 'L';
const char RIGHT_RISE_CHAR = 'R';
const char BRAKE_RISE_CHAR = 'B';
const char BRAKE_FALL_CHAR = 'b';
const char SIGNAL_CANCEL_CHAR = 'C';
const char HAZARD_CHAR = 'H'; // for future
const char FEEDBACK_CHAR = 'F';
const char BATTERY_LEVEL_CHAR = 't';

// ON/OFF chars for feedback
const char LEFT_ON_CHAR = 'L';
const char LEFT_OFF_CHAR = 'l';
const char RIGHT_ON_CHAR = 'R';
const char RIGHT_OFF_CHAR = 'r';
const char BRAKE_ON_CHAR = 'B';
const char BRAKE_OFF_CHAR = 'b';

// PWM LED values
const int BRK_LED_IDLE = 0x20;
const int BRK_LED_FULL = 0xFF;

// Blinking rate init
const int SIG_BLINK_DUR = 5000;

// RF Modules
const char T_ADDR[6] = "clie1";
const char R_ADDR[6] = "serv1";
const int MIRF_PAYLOAD_SIZE = 1;

// Battery constants
const int BATTERY_FULL = 861;
const int BATTERY_LOW = 685;
const int BATTERY_CRITICAL = 610;

////////////////////////////////////////////////////////////////////////////////
// VARIABLES  VARIABLES  VARIABLES  VARIABLES  VARIABLES  VARIABLES  VARIABLES//
////////////////////////////////////////////////////////////////////////////////
// LED state variables
int left_state = LOW;
int right_state = LOW;
int brake_state = LOW;
int sig_blink_count = 0;
int turn_signal_val = LOW;

// old LED state variables
int old_left_state = LOW;
int old_right_state = LOW;
int old_brake_state = LOW;

/////////////////////////////////////////////////////
// SETUP  SETUP  SETUP  SETUP  SETUP  SETUP  SETUP //
/////////////////////////////////////////////////////
void setup() {
  Serial.begin(BAUDRATE);
//  Serial.println("AURA DISPLAY ACTIVATING...");
  
  // Pin initializations
  pinMode(LEFT_LED, OUTPUT);
  pinMode(RIGHT_LED, OUTPUT);
  pinMode(BRAKE_LED, OUTPUT);
  
  // Check battery state
  int battery_value = analogRead(BATTERY_PIN);
  if(battery_value <= BATTERY_CRITICAL){
    
  }else if(battery_value <= BATTERY_LOW){
    
  }else{
    
  }
  
  // Mirf Setup
  Mirf.spi = &MirfHardwareSpi;
  Mirf.cePin = CE_PIN;
  Mirf.csnPin = CSN_PIN;
  Mirf.init();
  Mirf.setRADDR((byte *) R_ADDR);
  Mirf.setTADDR((byte *) T_ADDR);
  Mirf.payload = MIRF_PAYLOAD_SIZE;
  Mirf.config();
  
//  Serial.println("AURA DISPLAY ACTIVATED");
}
// END SETUP

////////////////////////////////////////////////////
// LOOP  LOOP  LOOP  LOOP  LOOP  LOOP  LOOP  LOOP //
////////////////////////////////////////////////////
void loop(){

  // State machine reading from serial
  if(!Mirf.isSending() && Mirf.dataReady()){
//    Serial.println("Got packet");
    char input;
    Mirf.getData((byte *) &input);
//    Serial.println(input);

    switch ( input ){
      case BRAKE_RISE_CHAR:
        brake_state = HIGH; break;

      case BRAKE_FALL_CHAR:
        brake_state = LOW; break;

      case LEFT_RISE_CHAR:
        left_state = HIGH; right_state = LOW; break;

      case RIGHT_RISE_CHAR:
        left_state = LOW; right_state = HIGH; break;

      case SIGNAL_CANCEL_CHAR:
        left_state = LOW; right_state = LOW; break;

      case HAZARD_CHAR:
        left_state = HIGH; right_state = HIGH; break;

      case BATTERY_LEVEL_CHAR:{
        int battery_value = analogRead(BATTERY_PIN);    
        Serial.print("Battery: ");
        Serial.print(battery_value);
        Serial.print(" which is: ");
        Serial.print(100.0*battery_value/((float) BATTERY_FULL)); // 861 = 4.2V = full charge
        Serial.println("%"); 
        break;
      }        
      case FEEDBACK_CHAR:
//        sendState();
        break;
    }
  }

  if (sig_blink_count <= -1*SIG_BLINK_DUR)
    sig_blink_count = SIG_BLINK_DUR;
  else
    sig_blink_count--;
  
  // Blinking waveform
  if (sig_blink_count >= 0)
    turn_signal_val = HIGH;
  else
    turn_signal_val = LOW;

  if(left_state == HIGH) {
    digitalWrite(LEFT_LED, turn_signal_val);
    if(old_left_state == LOW)
      sig_blink_count = SIG_BLINK_DUR;
  }
  else {
    digitalWrite(LEFT_LED, LOW);
  }

  if(right_state == HIGH) {
    digitalWrite(RIGHT_LED, turn_signal_val);
    if(old_right_state == LOW)
      sig_blink_count = SIG_BLINK_DUR;
  }
  else {
    digitalWrite(RIGHT_LED, LOW);
  }

  if(brake_state == HIGH) {
      analogWrite(BRAKE_LED, BRK_LED_FULL);
      digitalWrite(ON_BOARD_LED, HIGH);
  }
  else {
    analogWrite(BRAKE_LED, BRK_LED_IDLE);
    digitalWrite(ON_BOARD_LED, LOW);
  }
  
  // update old values
  old_left_state  = left_state;
  old_right_state = right_state;
}// END LOOP

/*
// returns char array of stuff 
void sendState(){
  char state[FEEDBACK_CHAR_LENGTH]; 
  if(brake_state == HIGH)
    state[0] = BRAKE_ON_CHAR;
  else
    state[0] = BRAKE_OFF_CHAR;

  if(left_state == HIGH)
    state[1] = LEFT_ON_CHAR;
  else
    state[1] = LEFT_OFF_CHAR;

  if(right_state == HIGH)
    state[2] = RIGHT_ON_CHAR;
  else
    state[2] = RIGHT_OFF_CHAR;
    
  state[3] = '\0';
  Soft_Serial.write(state);
} // end sendState
*/
