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
int numRows=8;

int numColumns = numColumnRegisters*8;
int numOutputs = numColumns*numRows;


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
  ShiftMatrixPWM.SetAll(0);
  Serial.begin(9600);
}

void loop()
{    
  // Print information about the interrupt frequency, duration and load on your program
  ShiftMatrixPWM.SetAll(maxBrightness+1);
  ShiftMatrixPWM.PrintInterruptLoad();

  // Fade in and fade out all outputs one by one fast. Usefull for testing your circuit
//  ShiftMatrixPWM.OneByOneFast();

  for(int x=0; x<numRows; x++){
    for(int y=0; y<numColumns; y++){
      // Fade in all outputs
      for(int j=maxBrightness+1;j>0;j--){
        ShiftMatrixPWM.SetOne(x,y,j);
        delay(2);
      }
      // Fade out all outputs
      for(int j=0;j<=maxBrightness+1;j++){
        ShiftMatrixPWM.SetOne(x,y,j);
        delay(2);
      }
    }
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
  //  A moving rainbow for RGB leds:
//  rgbLedRainbow(numOutputs/3, 5, 3, maxBrightness, numColumns/3); // Fast, over all LED's, 1 column wide rainbow
//  rgbLedRainbow(numOutputs/3, 10, 3, maxBrightness, numOutputs/3); //slower, over all LED's, as wide as the whole matrix

  // Fade in and fade out all outputs slowly. Usefull for testing your circuit
//  ShiftMatrixPWM.OneByOneSlow();  

}
