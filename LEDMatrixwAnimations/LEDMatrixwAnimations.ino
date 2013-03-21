#include <QueueList.h>
#include <FlexiTimer2.h>

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 4 is connected to the DataIn 
 pin 2 is connected to the CLK 
 pin 3 is connected to LOAD 
 We have only a single MAX72XX.
 */
//LedControl lc=LedControl(4,2,3,1);

// ShiftPWM stuffs
const int ShiftPWM_latchPin=10;
const bool ShiftPWM_invertOutputs = true;
const bool ShiftPWM_balanceLoad = false;
#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!
unsigned char maxBrightness = 31;
unsigned char pwmFrequency = 150;
int numRegisters = 1;
int numOutputs = numRegisters*8;

unsigned long startTime = 0; // start time for the chosen fading mode

// Demux pins
const int DEMUX_ADDR_A_PIN = 2;
const int DEMUX_ADDR_B_PIN = 3;
const int DEMUX_ADDR_C_PIN = 4;
const int DEMUX_ENABLE_PIN = 5;

// Demux variables
volatile char current_row = 0;
float dutyCycle = 1;

/* we always wait a bit between updates of the display */
unsigned long delaytime=500;
const int DELAYVAL = 42;

// State machine
enum SignalState {IDLE, LEFT, RIGHT};
SignalState current_state = IDLE;

// Global LED Modifiers
byte brake_modifier = B11111111;
const byte BRAKE_ON_MODIFIER = B00000000;
const byte BRAKE_OFF_MODIFIER = B11111111;
int phase_number = 0;
QueueList <byte> display_queue;
volatile byte display_buffer[8];

///////////////////////////////////////////////////////////////////////////////
// Setup  Setup  Setup  Setup  Setup  Setup  Setup  Setup  Setup  Setup  //////
///////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);

  // Set demux pins
  pinMode(DEMUX_ADDR_A_PIN,OUTPUT); // Addr A
  pinMode(DEMUX_ADDR_B_PIN,OUTPUT); // Addr B
  pinMode(DEMUX_ADDR_C_PIN,OUTPUT); // Addr C 
  pinMode(DEMUX_ENABLE_PIN,OUTPUT); // Enable Inputs
  digitalWrite(DEMUX_ENABLE_PIN,HIGH);
  
  // Sets the number of 8-bit registers that are used.
  ShiftPWM.SetAmountOfRegisters(numRegisters);

  // SetPinGrouping allows flexibility in LED setup. 
  // If your LED's are connected like this: RRRRGGGGBBBBRRRRGGGGBBBB, use SetPinGrouping(4).
  ShiftPWM.SetPinGrouping(1); //This is the default, but I added here to demonstrate how to use the funtion
  
  ShiftPWM.Start(pwmFrequency,maxBrightness);
  
  // Timer shit
  FlexiTimer2::set(1, 1.0/5, displaySlide);
  FlexiTimer2::start();
  
  current_state = IDLE;
} // end Setup

void displaySlide(){ // cycles through the rows
  // Set row to display
  digitalWrite(DEMUX_ADDR_A_PIN, current_row & B00000001);  
  digitalWrite(DEMUX_ADDR_B_PIN, (current_row >> 1) & B00000001);
  digitalWrite(DEMUX_ADDR_C_PIN, (current_row >> 2) & B00000001);
    
  // display column from display buffer
//  Serial.print("Row: ");
//  Serial.print(current_row,DEC);
//  Serial.print(" ");
//  Serial.println(display_buffer[current_row],BIN);
  for(int col=0; col<7; col++)
    ShiftPWM.SetOne(col,((display_buffer[current_row]) >> col & B00000001)*maxBrightness + 1);
  
  // Update current row
  if(current_row >= 7)
    current_row = 0;
  else
    current_row++;
}// end displaySlide

// Displays slide in front of queue
void pushSlideToBuffer(){
  for( int row = 0; row < 8; row++)
    display_buffer[row] = display_queue.pop() ^ brake_modifier;
}

void flushDisplayQueue(){
  // Flush display_queue
  while(!display_queue.isEmpty()){
    display_queue.pop();
  }
}

void loadIdleToQueue() {
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);  
  display_queue.push((byte)0);
  display_queue.push((byte)0);
}

void loadRightToQueue(){
  // Clear queue
  flushDisplayQueue();
  
  // Slide 1
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);  
  display_queue.push((byte)0);
  
  // Slide 2
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);
  
  // Slide 3
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  
  // Slide 4
  display_queue.push(B00111100);
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  
  // Slide 5
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  
  // Slide 6
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  
  // Slide 7
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);  
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  
  // Slide 8
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);  
  display_queue.push((byte)0);
  display_queue.push((byte)0);
}

void loadLeftToQueue(){
  
  // Clear queue
  flushDisplayQueue();
  
  // Slide 1
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);  
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  
  // Slide 2
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  
  // Slide 3
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  
  // Slide 4
  display_queue.push(B00111100);
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  
  // Slide 5
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  
  // Slide 6
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);
  
  // Slide 7
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);  
  display_queue.push((byte)0);
  
  // Slide 8
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);  
  display_queue.push((byte)0);
  display_queue.push((byte)0);
}

// State machine
void signalStateMachine(){
  
  if(Serial.available()){
//    Serial.print("Current state is ");
//    Serial.print(current_state);
    char input = Serial.read();
    switch(input) {
      case 'B':
        brake_modifier = BRAKE_ON_MODIFIER;
        break;
      case 'b':
        brake_modifier = BRAKE_OFF_MODIFIER;
        break;
      case 'L':
        current_state = LEFT;
        break;
      case 'R':
        current_state = RIGHT;
        break;
      case 'C':
        current_state = IDLE;
        break;
      case 'p':
        Serial.println("");
        for(int i=0; i<8; i++){
          Serial.println(display_buffer[i],BIN);
        }
        break;  
    }
//    Serial.print(", changed to ");
//    Serial.println(current_state);
  } // end Serial switch
}

///////////////////////////////////////////////////////////////////////////////
// Loop  Loop  Loop  Loop  Loop  Loop  Loop  Loop  Loop  Loop  Loop  Loop  ////
///////////////////////////////////////////////////////////////////////////////
void loop() { 
  // Check for inputs, then change states
  signalStateMachine();
//  Serial.print("Size of queue is ");
//  Serial.println(display_queue.count());
  // Refill if empty
  if(display_queue.isEmpty()){
//    Serial.print("Queue is empty, ");
    switch(current_state){
      case IDLE:
//        Serial.println("load idle slides...");
        loadIdleToQueue();
        break;
      case LEFT:
//        Serial.println("load left slides...");
        loadLeftToQueue();
        break;
      case RIGHT:
//        Serial.println("load right slides...");
        loadRightToQueue();
        break;
    }
  }
  
  // push current slide to display buffer
  pushSlideToBuffer();
  
  delay(DELAYVAL);
  
} // end loop

