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

unsigned char max_brightness = 31;
unsigned char pwm_frequency = 60;
int num_column_registers = 1;
int num_rows=7;

int num_columns = num_column_registers*8;
int numOutputs = num_columns*num_rows;

/* we always wait a bit between updates of the display */
const int DELAYVAL = 100;
//
//// State machine
////SignalState current_state = IDLE;
//
//// Global LED Modifiers
//byte brake_modifier = B11111111;
//const byte BRAKE_ON_MODIFIER = B00000000;
//const byte BRAKE_OFF_MODIFIER = B11111111;
//QueueList <unsigned char> display_queue;

// Gamma LUT function (gamma = 1/5)
// negative values mean complementary brightness
char gammaLUT(unsigned char brightness){
  static const unsigned char lookup[32] = {
     0,  5,  8, 10, 12, 14, 15, 17, 18, 19,
    20, 21, 21, 22, 23, 24, 24, 25, 26, 26,
    27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 
    32, 32
  };
  if(brightness >=0)
    return lookup[brightness];
  else // complementary colours
    return lookup[32 + brightness];
}// end Gamma LUT

const unsigned char IDLE_SLIDES[35][7] = {
  // Slide 1
  {32, 32, 32, 32, 32, 32, 32},
  {32, 32, 32, 32, 32, 32, 32},
  {32, 32, 32, 28, 32, 32, 32},
  {32, 32, 28, 24, 28, 32, 32},
  {32, 32, 32, 28, 32, 32, 32},
  {32, 32, 32, 32, 32, 32, 32},
  {32, 32, 32, 32, 32, 32, 32},
  
  // Slide 2
  {32, 32, 32, 32, 32, 32, 32},
  {32, 32, 32, 28, 32, 32, 32},
  {32, 32, 28, 24, 28, 32, 32},
  {32, 28, 24, 20, 24, 28, 32},
  {32, 32, 28, 24, 28, 32, 32},
  {32, 32, 32, 28, 32, 32, 32},
  {32, 32, 32, 32, 32, 32, 32},
  
  // Slide 3
  {32, 32, 32, 28, 32, 32, 32},
  {32, 32, 28, 24, 28, 32, 32},
  {32, 28, 24, 20, 24, 28, 32},
  {28, 24, 20, 16, 20, 24, 28},
  {32, 28, 24, 20, 24, 28, 32},
  {32, 32, 28, 24, 28, 32, 32},
  {32, 32, 32, 28, 32, 32, 32},
  
  // Slide 4
  {32, 32, 32, 32, 32, 32, 32},
  {32, 32, 32, 28, 32, 32, 32},
  {32, 32, 28, 24, 28, 32, 32},
  {32, 28, 24, 20, 24, 28, 32},
  {32, 32, 28, 24, 28, 32, 32},
  {32, 32, 32, 28, 32, 32, 32},
  {32, 32, 32, 32, 32, 32, 32},
  
  // Slide 5
  {32, 32, 32, 32, 32, 32, 32},
  {32, 32, 32, 32, 32, 32, 32},
  {32, 32, 32, 28, 32, 32, 32},
  {32, 32, 28, 24, 28, 32, 32},
  {32, 32, 32, 28, 32, 32, 32},
  {32, 32, 32, 32, 32, 32, 32},
  {32, 32, 32, 32, 32, 32, 32}
};// END IDLE SLIDES
  
//  {30, 28, 24, 16, 24, 28, 30},
//  {28, 24, 16,  8, 16, 24, 28},
//  {24, 16,  8,  4,  8, 16, 24},
//  {16,  8,  4,  0,  4,  8, 16},
//  {24, 16,  8,  4,  8, 16, 24},
//  {28, 24, 16,  8, 16, 24, 28},
//  {30, 28, 24, 16, 24, 28, 30}};

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
  SPI.setClockDivider(SPI_CLOCK_DIV4); 
  SPI.begin(); 

  ShiftMatrixPWM.SetMatrixSize(num_rows, num_column_registers);
  ShiftMatrixPWM.Start(pwm_frequency,max_brightness);  
  ShiftMatrixPWM.SetAll(max_brightness+1);
  
//  current_state = IDLE;
} // end Setup

void displaySlide(){ // Sends a slide to ShiftMatrixPWM buffer
  int row;
  
  // Set Row 1 first because of stupid bug (row 1 = row 0)
  for(row = 1; row < num_rows; row++){
    for(int col = 0; col < num_columns-1; col++){
//      ShiftMatrixPWM.SetOne(row,col,gammaLUT(display_queue.pop()));
    }
  }
  
  // Now display last row (row 0 = row 6)
  row = 0;
  for(int col = 0; col < num_columns-1; col++){
//    ShiftMatrixPWM.SetOne(row,col,gammaLUT(display_queue.pop()));
  }

}// end displaySlide

void flushDisplayQueue(){
  // Flush display_queue
  while(!display_queue.isEmpty()){
    display_queue.pop();
  }
}

void loadIdleToQueue() {
  Serial.print("Loading idle slides... ");
  for(int row=0; row<35; row++){
    Serial.print("Loading slide ");
    Serial.println(row);
    for(int col=0; col<7; col++)
      display_queue.push(IDLE_SLIDES[row][col]);
  }
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
  
  // Slide 2
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  
  // Slide 3
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  
  // Slide 4
  display_queue.push(B00111100);
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  
  // Slide 5
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  
  // Slide 6
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);
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
  
  // Slide 8
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);  
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
  
  // Slide 2
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);
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
  
  // Slide 4
  display_queue.push(B00111100);
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  
  // Slide 5
  display_queue.push(B00011000);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  
  // Slide 6
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  
  // Slide 7
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);  
  
  // Slide 8
  display_queue.push((byte)0);
  display_queue.push((byte)0);
  display_queue.push(B00011000);
  display_queue.push(B00111100);
  display_queue.push(B00111100);
  display_queue.push(B00011000);  
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
//        current_state = LEFT;
        break;
      case 'R':
//        current_state = RIGHT;
        break;
      case 'C':
//        current_state = IDLE;
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
  Serial.print("Size of queue is ");
  Serial.println(display_queue.count());
  // Refill if empty
  if(display_queue.isEmpty()){
    Serial.print("Queue is empty, ");
//    switch(current_state){
//      case IDLE:
//        Serial.println("load idle slides...");
//        loadIdleToQueue();
//        break;
//      case LEFT:
////        Serial.println("load left slides...");
//        loadLeftToQueue();
//        break;
//      case RIGHT:
////        Serial.println("load right slides...");
//        loadRightToQueue();
//        break;
//    }
  }
  
  // push current slide to display buffer
  displaySlide();
  
  delay(DELAYVAL);
  
} // end loop

