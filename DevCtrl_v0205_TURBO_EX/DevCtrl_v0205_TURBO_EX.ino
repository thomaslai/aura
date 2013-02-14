#include <SoftwareSerial.h>
#include <avr/interrupt.h>

SoftwareSerial Soft_Serial(11, 12); // RX, TX
const int BAUDRATE = 9600;
const int DELAYVAL = 50;
const int FEEDBACK_COUNTER_LOOPS = 100; // 1 second in milliseconds

// constants won't change. They're used here to 
// set pin numbers:
const int BRAKE_PIN = 2;
const int LEFT_PIN = 7;     // the number of the pushbutton pin
const int RIGHT_PIN = 8;
const int CANCEL_PIN = 10; // signal cancelling (left and right)

// LED Output pins
const int BRAKE_CHECK_LED = 3;
const int LEFT_CHECK_LED = 4;
const int RIGHT_CHECK_LED = 5;
const int ON_BOARD_LED = 13;

// Soft Serial Char Mapping
const char BRAKE_RISE_CHAR = 'B';
const char BRAKE_FALL_CHAR = 'b';
const char LEFT_RISE_CHAR = 'L';
const char RIGHT_RISE_CHAR = 'R';
const char SIGNAL_CANCEL_CHAR = 'C';
const char HAZARD_CHAR = 'H'; // for future
const char FEEDBACK_CHAR = 'F';

// ON/OFF chars for feedback
const char LEFT_ON_CHAR = 'L';
const char LEFT_OFF_CHAR = 'l';
const char RIGHT_ON_CHAR = 'R';
const char RIGHT_OFF_CHAR = 'r';
const char BRAKE_ON_CHAR = 'B';
const char BRAKE_OFF_CHAR = 'b';

// PWM LED values
const int BRAKE_LED_IDLE = 0x20;
const int BRAKE_LED_FULL = 0xFF;

// pin state variables
int brake_pin_val  = LOW;
int left_pin_val   = LOW;         // variable for reading the pushbutton status
int right_pin_val  = LOW;
int cancel_pin_val = LOW;

// old pin state variables
int old_brake_pin_val  = LOW;
int old_left_pin_val   = LOW;         // variable for reading the pushbutton status
int old_right_pin_val  = LOW;
int old_cancel_pin_val = LOW;

// rise/fall state booleans 0 = NC, 1 = rising, -1 = falling
int brake_state = 0;
int left_state = 0;
int right_state = 0;
int cancel_state = 0;
int count = 0;

// Feedback check booleans
char brake_is_on = BRAKE_OFF_CHAR;
char left_is_on = LEFT_OFF_CHAR;
char right_is_on = RIGHT_OFF_CHAR;

/////////////////////////////////////////////////////
// SETUP  SETUP  SETUP  SETUP  SETUP  SETUP  SETUP //
/////////////////////////////////////////////////////
void setup() {
//  Serial.begin(BAUDRATE);
//  Serial.println("AURA ACTIVATED");
  Soft_Serial.begin(BAUDRATE);
  
  // Initializing pin ios
  pinMode(LEFT_PIN, INPUT);
  pinMode(RIGHT_PIN, INPUT);
  pinMode(BRAKE_PIN, INPUT);
  pinMode(CANCEL_PIN, INPUT);
  
  pinMode(LEFT_CHECK_LED, OUTPUT);
  pinMode(RIGHT_CHECK_LED, OUTPUT);
  pinMode(BRAKE_CHECK_LED, OUTPUT);
} // end setup

////////////////////////////////////////////////////
// LOOP  LOOP  LOOP  LOOP  LOOP  LOOP  LOOP  LOOP //
////////////////////////////////////////////////////
void loop(){
  
  // Read fresh pin states
  left_pin_val   = digitalRead(LEFT_PIN);
  right_pin_val  = digitalRead(RIGHT_PIN);
  brake_pin_val  = digitalRead(BRAKE_PIN);
  cancel_pin_val = digitalRead(CANCEL_PIN);
  
  // check rise/fall states
  brake_state   = edgeDetector(old_brake_pin_val, brake_pin_val);
  left_state    = edgeDetector(old_left_pin_val, left_pin_val);
  right_state   = edgeDetector(old_right_pin_val, right_pin_val);
  cancel_state  = edgeDetector(old_cancel_pin_val, cancel_pin_val);
  
  // Brake states check and serial send
  if( brake_state == 1 ){
    brake_is_on = BRAKE_ON_CHAR;
    Soft_Serial.write(BRAKE_RISE_CHAR);
    digitalWrite( ON_BOARD_LED, HIGH);
    analogWrite( BRAKE_CHECK_LED, BRAKE_LED_FULL);
    delay(DELAYVAL);
  }  else if ( brake_state == -1 ){
    brake_is_on = BRAKE_OFF_CHAR;
    Soft_Serial.write(BRAKE_FALL_CHAR);
    digitalWrite(ON_BOARD_LED,LOW);
    analogWrite( BRAKE_CHECK_LED, BRAKE_LED_IDLE);
    delay(DELAYVAL);
  }
  
  // Left / right states check and serial send
  if( left_state == 1 ){
    left_is_on = LEFT_ON_CHAR;
    right_is_on = RIGHT_OFF_CHAR;
    Soft_Serial.write(LEFT_RISE_CHAR);
    digitalWrite(LEFT_CHECK_LED,HIGH);
    digitalWrite(RIGHT_CHECK_LED,LOW);
    delay(DELAYVAL);
  }  else if ( right_state == 1 ){
    left_is_on = LEFT_OFF_CHAR;
    right_is_on = RIGHT_ON_CHAR;
    Soft_Serial.write(RIGHT_RISE_CHAR);
    digitalWrite(LEFT_CHECK_LED,LOW);
    digitalWrite(RIGHT_CHECK_LED,HIGH);
    delay(DELAYVAL);
  }  else if ( cancel_state == 1 ){
    left_is_on = LEFT_OFF_CHAR;
    right_is_on = RIGHT_OFF_CHAR;
    Soft_Serial.write(SIGNAL_CANCEL_CHAR);
    digitalWrite(LEFT_CHECK_LED,LOW);
    digitalWrite(RIGHT_CHECK_LED,LOW);
    delay(DELAYVAL);
  }
  
//  if(count > FEEDBACK_COUNTER_LOOPS){
//    count = 0;
//    checkFeedback();
//  }
//  count++;
  
  // Update old values
  old_left_pin_val   = left_pin_val;
  old_right_pin_val  = right_pin_val;
  old_brake_pin_val  = brake_pin_val;
  old_cancel_pin_val = cancel_pin_val;
  

} // end loop

// edgeDetector
int edgeDetector(int old_value, int new_value){
  if(new_value > old_value){
    return 1;
  }else if(new_value < old_value){
    return -1;
  }else{ // no change
    return 0;
  }
} // end edgeDetector

void checkFeedback(){
  char brake_feedback;
  char left_feedback;
  char right_feedback;
  
  Soft_Serial.write(FEEDBACK_CHAR);
  while(!Soft_Serial.available());
  brake_feedback = Soft_Serial.read();
  while(!Soft_Serial.available());
  left_feedback = Soft_Serial.read();
  while(!Soft_Serial.available());
  right_feedback = Soft_Serial.read();
  
//  Soft_Serial.write(brake_feedback);
//  Soft_Serial.println(); Soft_Serial.write(left_feedback);
//  Soft_Serial.println(); Soft_Serial.write(right_feedback);
//  Soft_Serial.println();
  
  if(brake_is_on != brake_feedback){
    Soft_Serial.write(brake_is_on);
  }
  
  if(left_is_on != left_feedback){
    Soft_Serial.write(left_is_on);
  }
  
  if(right_is_on != right_feedback){
    Soft_Serial.write(right_is_on);
  }
  
}
