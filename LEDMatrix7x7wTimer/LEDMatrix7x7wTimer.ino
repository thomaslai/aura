/************************************************************************************************************************************
 * ShiftPWM non-blocking RGB fades example, (c) Elco Jacobs, updated August 2012.
 *
 * This example for ShiftPWM shows how to control your LED's in a non-blocking way: no delay loops.
 * This example receives a number from the serial port to set the fading mode. Instead you can also read buttons or sensors.
 * It uses the millis() function to create fades. The block fades example might be easier to understand, so start there.
 * 
 * Please go to www.elcojacobs.com/shiftpwm for documentation, fuction reference and schematics.
 * If you want to use ShiftPWM with LED strips or high power LED's, visit the shop for boards.
 ************************************************************************************************************************************/
 
//#include <Servo.h> <-- If you include Servo.h, which uses timer1, ShiftPWM will automatically switch to timer2

// Clock and data pins are pins from the hardware SPI, you cannot choose them yourself.
// Data pin is MOSI (Uno and earlier: 11, Leonardo: ICSP 4, Mega: 51, Teensy 2.0: 2, Teensy 2.0++: 22) 
// Clock pin is SCK (Uno and earlier: 13, Leonardo: ICSP 3, Mega: 52, Teensy 2.0: 1, Teensy 2.0++: 21)

// You can choose the latch pin yourself.
const int ShiftPWM_latchPin=10;

// ** uncomment this part to NOT use the SPI port and change the pin numbers. This is 2.5x slower **
// #define SHIFTPWM_NOSPI
// const int ShiftPWM_dataPin = 7;
// const int ShiftPWM_clockPin = 8;


// If your LED's turn on if the pin is low, set this to true, otherwise set it to false.
const bool ShiftPWM_invertOutputs = true;

// You can enable the option below to shift the PWM phase of each shift register by 8 compared to the previous.
// This will slightly increase the interrupt load, but will prevent all PWM signals from becoming high at the same time.
// This will be a bit easier on your power supply, because the current peaks are distributed.
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!
#include <FlexiTimer2.h>

// Here you set the number of brightness levels, the update frequency and the number of shift registers.
// These values affect the load of ShiftPWM.
// Choose them wisely and use the PrintInterruptLoad() function to verify your load.
unsigned char maxBrightness = 31;
unsigned char pwmFrequency = 150;
int numRegisters = 1;
int numOutputs = numRegisters*8;
int numRGBLeds = numRegisters*8/3;
int fadingMode = 2; //start with all LED's off.

unsigned long startTime = 0; // start time for the chosen fading mode

// Demux pins
const int DEMUX_ADDR_A_PIN = 2;
const int DEMUX_ADDR_B_PIN = 3;
const int DEMUX_ADDR_C_PIN = 4;
const int DEMUX_ENABLE_PIN = 5;

// Demux variables
volatile char CURRENT_ROW = 0;
float dutyCycle = 1;

void setup()   {                
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
  printInstructions();
  
  // Timer shit
  FlexiTimer2::set(1, 1.0/5000, cycleRows);
  FlexiTimer2::start();
}

void loop()
{    
  if(Serial.available()){ // Serial menu
    if(Serial.peek() == 'l'){
      // Print information about the interrupt frequency, duration and load on your program
      ShiftPWM.PrintInterruptLoad();
      Serial.flush();
    }
    else{
      fadingMode = Serial.parseInt(); // read a number from the serial port to set the mode
      Serial.print("Mode set to "); 
      Serial.print(fadingMode); 
      Serial.print(": ");
      startTime = millis();
      switch(fadingMode){
      case 0:
        Serial.println("All LED's off"); break;
      case 1:
        Serial.println("Fade in and out one by one"); break;
      case 2:
        Serial.println("Fade in and out all LED's"); break;
      case 3:
        Serial.println("Fade in and out 2 LED's in parallel"); break;
      case 4:
        Serial.println("Please enter dutycycle value from 0 to 1:");
        while(!Serial.available());
        dutyCycle = Serial.parseFloat();
        Serial.print("Setting LED Matrix to duty cycle:");
        Serial.println(dutyCycle); break;
      default:
        Serial.println("Unknown mode!");
        break;
      }
    } 
  } // End serial menu
  
  unsigned char brightness;
  switch(fadingMode){
  case 0:
    // Turn all LED's off.
    ShiftPWM.SetAll(0);
    break;
  case 1:
    oneByOne(); break;
  case 2:
    inOutAll(); break;
  case 3:
    inOutTwoLeds(); break;
  case 4:
    setDutyCycle(); break;
  default:
    Serial.println("Unknown Mode!");
    delay(1000);
    break;
  }
  
}// End Loop

void cycleRows()
{
  // Set row to display
//  Serial.print((int) CURRENT_ROW & B00000001);
//  Serial.print((int) (CURRENT_ROW >> 1) & B00000001);
//  Serial.println((int) (CURRENT_ROW >> 2) & B00000001);
  digitalWrite(DEMUX_ADDR_A_PIN, CURRENT_ROW & B00000001);
  digitalWrite(DEMUX_ADDR_B_PIN, (CURRENT_ROW >> 1) & B00000001);
  digitalWrite(DEMUX_ADDR_C_PIN, (CURRENT_ROW >> 2) & B00000001);
  if(CURRENT_ROW >= 6)
    CURRENT_ROW = 0;
  else
    CURRENT_ROW++;
}// end cycleRows

void setDutyCycle(){
    ShiftPWM.SetAll(dutyCycle*maxBrightness);
}

void oneByOne(void){ // Fade in and fade out all outputs one at a time
  unsigned char brightness;
  unsigned long fadeTime = 300;
  unsigned long loopTime = numOutputs*fadeTime*2;
  unsigned long time = millis()-startTime;
  unsigned long timer = time%loopTime;
  unsigned long currentStep = timer%(fadeTime*2);

  int activeLED = timer/(fadeTime*2);

  if(currentStep <= fadeTime ){
    brightness = currentStep*maxBrightness/fadeTime; ///fading in
  }
  else{
    brightness = maxBrightness-(currentStep-fadeTime)*maxBrightness/fadeTime; ///fading out;
  }
  ShiftPWM.SetAll(0);
  ShiftPWM.SetOne(activeLED, brightness);
}

void inOutTwoLeds(void){ // Fade in and out 2 outputs at a time
  unsigned long fadeTime = 500;
  unsigned long loopTime = numOutputs*fadeTime;
  unsigned long time = millis()-startTime;
  unsigned long timer = time%loopTime;
  unsigned long currentStep = timer%fadeTime;

  int activeLED = timer/fadeTime;
  unsigned char brightness = currentStep*maxBrightness/fadeTime;

  ShiftPWM.SetAll(0);
  ShiftPWM.SetOne((activeLED+1)%numOutputs,brightness);
  ShiftPWM.SetOne(activeLED,maxBrightness-brightness);
}

void inOutAll(void){  // Fade in all outputs
  unsigned char brightness;
  unsigned long fadeTime = 2500;
  unsigned long time = millis()-startTime;
  unsigned long currentStep = time%(fadeTime*2);

  if(currentStep <= fadeTime ){
    brightness = currentStep*maxBrightness/fadeTime; ///fading in
  }
  else{
    brightness = maxBrightness-(currentStep-fadeTime)*maxBrightness/fadeTime; ///fading out;
  }
  
  ShiftPWM.SetAll(0.8*brightness);
  
}

void printInstructions(void){
  Serial.println("---- ShiftPWM Non-blocking fades demo ----");
  Serial.println("");
  
  Serial.println("Type 'l' to see the load of the ShiftPWM interrupt (the % of CPU time the AVR is busy with ShiftPWM)");
  Serial.println("");
  Serial.println("Type any of these numbers to set the demo to this mode:");
  Serial.println("  0. All LED's off");
  Serial.println("  1. Fade in and out one by one");
  Serial.println("  2. Fade in and out all LED's");
  Serial.println("  3. Fade in and out 2 LED's in parallel");
  Serial.println("  4. Set custom PWM duty cycle");
  Serial.println("");
  Serial.println("Type 'm' to see this info again");  
  Serial.println("");
  Serial.println("----");
}






