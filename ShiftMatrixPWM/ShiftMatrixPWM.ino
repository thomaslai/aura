/******************************************************************************
 * //Comments yet to to be updated 
 * 
 * (c) Elco Jacobs, Sept 2011.
 * 
 *****************************************************************************/
//#include <Servo.h>
#include <SPI.h>
//#include "hsv2rgb.h"


//Data pin is MOSI (atmega168/328: pin 11. Mega: 51) 
//Clock pin is SCK (atmega168/328: pin 13. Mega: 52)
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
int numRows=7;

int numColumns = numColumnRegisters*8;
int numOutputs = numColumns*numRows;

// Gamma LUT
uint8_t gammaLUT(uint8_t brightness){
  static const uint8_t lookup[32] = {
     0,  5,  8, 10, 12, 14, 15, 17, 18, 19,
    20, 21, 21, 22, 23, 24, 24, 25, 26, 26,
    27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 
    32, 32
  };
  return lookup[brightness];
}// end Gamma LUT

void setup()   {                
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
  Serial.begin(9600);
}

void loop()
{    
  // Print information about the interrupt frequency, duration and load on your program
  ShiftMatrixPWM.PrintInterruptLoad();

  // Fade in and fade out all outputs one by one fast. Usefull for testing your circuit
//  ShiftMatrixPWM.OneByOneFast();

//  for(int x=0; x<numRows; x++){
//    for(int y=0; y<numColumns; y++){
//      // Fade in all outputs
//      for(int j=maxBrightness+1;j>0;j--){
//        ShiftMatrixPWM.SetOne(x,y,j);
//        delay(2);
//      }
//      // Fade out all outputs
//      for(int j=0;j<=maxBrightness+1;j++){
//        ShiftMatrixPWM.SetOne(x,y,j);
//        delay(2);
//      }
//    }
//  }
  
  // Fade in all outputs
//  Serial.println("Fade in all outputs");
  unsigned int count = 0;
  while(true){
//    ShiftMatrixPWM.SetAll(gammaLUT(maxBrightness/2 + maxBrightness*sin(count/200)/2));
//    count++;
//    delay(50);
    for(int j=maxBrightness;j>0;j--){
      ShiftMatrixPWM.SetAll(gammaLUT(j));
      delay(30);
    }
    delay(30);
    // Fade out all outputs
    for(int j=0;j<=maxBrightness;j++){
      ShiftMatrixPWM.SetAll(gammaLUT(j));
      delay(30);
    }
    delay(30);
  }
  
/*

  // Fade in and out 2 outputs at a time
  for(int row=0;row<numRows;row++){
    for(int col=0;col<numColumns-1;col++){
      ShiftMatrixPWM.SetAll(0);
      for(int brightness=0;brightness<maxBrightness;brightness++){
        ShiftMatrixPWM.SetOne(row, col+1,brightness);
        ShiftMatrixPWM.SetOne(row,col,maxBrightness-brightness);
        delay(1);
      }
    }
  }
*/

  // Fade in and fade out all outputs slowly. Usefull for testing your circuit
//  ShiftMatrixPWM.OneByOneSlow();  

}
