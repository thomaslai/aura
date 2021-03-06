#include <QueueList.h>
#include <SPI.h>

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
const int ShiftMatrixPWM_columnLatchPin=9;
const int ShiftMatrixPWM_rowDataPin=5;
const int ShiftMatrixPWM_rowClockPin=6;
const int ShiftMatrixPWM_rowLatchPin=3;

const bool ShiftMatrixPWM_invertColumnOutputs = 1; // if invertColumnOutputs is 1, outputs will be active low. Usefull for common anode RGB led's.

const bool ShiftMatrixPWM_invertRowOutputs = 1; // if invertOutputs is 1, outputs will be active low. Used for PNP transistors.

#include <ShiftMatrixPWM.h>   // include ShiftMatrixPWM.h after setting the pins!

unsigned char maxBrightness = 31;
unsigned char pwmFrequency = 60;
int numColumnRegisters = 1;
int numRows=8;

int numColumns = numColumnRegisters*8;
int numOutputs = numColumns*numRows;

/* we always wait a bit between updates of the display */
//unsigned long delaytime=500;
const int DELAYVAL = 100;

// State machine
enum SignalState {IDLE, LEFT, RIGHT};
SignalState current_state = IDLE;

// Global LED Modifiers
byte brake_modifier = B11111111;
const byte BRAKE_ON_MODIFIER = B00000000;
const byte BRAKE_OFF_MODIFIER = B11111111;
int phase_number = 0;
QueueList <byte> display_queue;
//volatile byte display_buffer[8];

///////////////////////////////////////////////////////////////////////////////
// Setup  Setup  Setup  Setup  Setup  Setup  Setup  Setup  Setup  Setup  //////
///////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  pinMode(ShiftMatrixPWM_columnLatchPin, OUTPUT); 
  pinMode(ShiftMatrixPWM_rowDataPin, OUTPUT); 
  pinMode(ShiftMatrixPWM_rowClockPin, OUTPUT); 
  pinMode(ShiftMatrixPWM_rowLatchPin, OUTPUT); 
  
  SPI.setBitOrder(LSBFIRST);
  // SPI_CLOCK_DIV2 is only a tiny bit faster in sending out the last byte. 
  // SPI transfer and calculations overlap for the other bytes.
  SPI.setClockDivider(SPI_CLOCK_DIV2); 
  SPI.begin(); 

  ShiftMatrixPWM.SetMatrixSize(numRows, numColumnRegisters);
  ShiftMatrixPWM.Start(pwmFrequency,maxBrightness);  
  ShiftMatrixPWM.SetAll(maxBrightness+1);
  
  current_state = IDLE;
} // end Setup

void displaySlide(){ // Sends a slide to ShiftMatrixPWM buffer
  byte popped_row;
  for(int row = 0; row < 8; row++){
    popped_row = display_queue.pop() ^ brake_modifier;
    for(int col = 7; col >=0 ; col--){
      ShiftMatrixPWM.SetOne(row,col, 
      (popped_row >> col & B00000001)*(maxBrightness+1));
    }
  }

}// end displaySlide

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
  displaySlide();
  
  delay(DELAYVAL);
  
} // end loop

