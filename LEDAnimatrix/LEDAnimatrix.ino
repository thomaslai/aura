#include <QueueList.h>
#include <SPI.h>
#include "Animatrix.h"

// ShiftPWM stuffs
const int ShiftMatrixPWM_columnLatchPin=9;
const int ShiftMatrixPWM_rowDataPin=5;
const int ShiftMatrixPWM_rowClockPin=6;
const int ShiftMatrixPWM_rowLatchPin=3;

const bool ShiftMatrixPWM_invertColumnOutputs = 1; // if invertColumnOutputs is 1, 
                                                   // outputs will be active low. 
                                                   // Usefull for common anode RGB led's.

const bool ShiftMatrixPWM_invertRowOutputs = 1; // if invertOutputs is 1, 
                                                // outputs will be active low. 
                                                // Used for PNP transistors.

#include <ShiftMatrixPWM.h>   // include ShiftMatrixPWM.h after setting the pins!

unsigned char max_brightness = 31;
unsigned char pwm_frequency = 60;
int num_column_registers = 1;
int num_rows=7;

int num_columns = num_column_registers*8;
int numOutputs = num_columns*num_rows;

/* we always wait a bit between updates of the display */
const int DELAYVAL = 5000;

Animatrix::Animatrix matrix_animator;

// Gamma LUT function (gamma = 1/5)
// negative values mean complementary brightness
unsigned char gammaLUT(unsigned char brightness){
  static const unsigned char lookup[32] = {
     0,  5,  8, 10, 12, 14, 15, 17, 18, 19,
    20, 21, 21, 22, 23, 24, 24, 25, 26, 26,
    27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 
    32, 32
  };

  return lookup[brightness];
}// end Gamma LUT

// Loads next slide to shiftmatrixpwm
void loadNextSlide(unsigned char * slides){
  Serial.println("\nLoading next slide...");
  int row;
  
  // Set Row 1-6 first because of stupid bug (row 1 = row 0)
  for(row = 1; row < 7; row++){
    for(int col = 0; col < 7; col++){
      Serial.print(gammaLUT(*(slides+7*(row-1) + col)),DEC);
      ShiftMatrixPWM.SetOne(row,col,gammaLUT( *(slides+7*(row-1) + col)));
      Serial.print(" ");
    }
    Serial.println("");
  }
  
  // Now display last row (row 0 = row 6)
  row = 0;
  for(int col = 0; col < 7; col++){
    Serial.print(gammaLUT(*(slides+7*6 + col)),DEC);
    ShiftMatrixPWM.SetOne(row,col,gammaLUT( *(slides+7*6 + col)));
    Serial.print(" ");
  }
  Serial.println("");
}

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

  // Setup matrix animator
  matrix_animator = Animatrix();
  
  for(int i=0; i<32; i++)
    Serial.println(gammaLUT(i));
  
} // end Setup

///////////////////////////////////////////////////////////////////////////////
// Loop  Loop  Loop  Loop  Loop  Loop  Loop  Loop  Loop  Loop  Loop  Loop  ////
///////////////////////////////////////////////////////////////////////////////
void loop() { 
  // Check for inputs, then change states
  if(Serial.available())
    matrix_animator.changeState(Serial.read());
  
  loadNextSlide(matrix_animator.getNextSlide());
  
  delay(DELAYVAL);  
} // end loop

