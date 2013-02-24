#include <LedControl.h>
#include <QueueList.h>

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 4 is connected to the DataIn 
 pin 2 is connected to the CLK 
 pin 3 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(4,2,3,1);

/* we always wait a bit between updates of the display */
unsigned long delaytime=100;
const int DELAYVAL = 42;

// State machine
enum SignalState {IDLE, LEFT, RIGHT};
SignalState current_state = IDLE;

// Global LED Modifiers
byte brake_modifier = B00000000;
const byte BRAKE_ON_MODIFIER = B11111111;
const byte BRAKE_OFF_MODIFIER = B00000000;
int phase_number = 0;
QueueList <byte> display_queue;

///////////////////////////////////////////////////////////////////////////////
// Setup  Setup  Setup  Setup  Setup  Setup  Setup  Setup  Setup  Setup  //////
///////////////////////////////////////////////////////////////////////////////
void setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);
  
  Serial.begin(9600);
  
  current_state = IDLE;
}

// Displays slide in front of queue
void displaySlide(){
  for( int row = 0; row < 8; row++) {
    lc.setRow(0,row,display_queue.pop() ^ brake_modifier);
  }
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
    Serial.print("Current state is ");
    Serial.print(current_state);
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
    Serial.print(", changed to ");
    Serial.println(current_state);
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
  
  // Display current slide
  displaySlide();
  
  delay(DELAYVAL);
  
} // end loop

