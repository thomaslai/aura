#include "Animatrix.h"
///////////////////////////////////////////////////////////////////////////////
// Animation definitions  Animation definitions  Animation definitions  ///////
///////////////////////////////////////////////////////////////////////////////
unsigned char IDLE_SLIDES[35][7] = {
  // Slide 1
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 27, 23, 27, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31},
  
  // Slide 2
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 27, 23, 27, 31, 31},
  {31, 27, 23, 19, 23, 27, 31},
  {31, 31, 27, 23, 27, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31},
  
  // Slide 3
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 27, 23, 27, 31, 31},
  {31, 27, 23, 19, 23, 27, 31},
  {27, 23, 19, 15, 19, 23, 27},
  {31, 27, 23, 19, 23, 27, 31},
  {31, 31, 27, 23, 27, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  
  // Slide 4
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 27, 23, 27, 31, 31},
  {31, 27, 23, 19, 23, 27, 31},
  {31, 31, 27, 23, 27, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31},
  
  // Slide 5
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 27, 23, 27, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31}
};// end IDLE SLIDES

unsigned char LEFT_SLIDES[7][7] = {
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 27, 23, 27, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31}
};

unsigned char RIGHT_SLIDES[7][7] = {
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 27, 23, 27, 31, 31},
  {31, 31, 31, 27, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31},
  {31, 31, 31, 31, 31, 31, 31}
};

///////////////////////////////////////////////////////////////////////////////
// Functions  Functions  Functions  Functions  Functions  Functions  Functions/
///////////////////////////////////////////////////////////////////////////////

// Animatrix constructor
Animatrix::Animatrix(){
  _current_slides = &IDLE_SLIDES[0][0];
  _brake_on = false;
  _slide_number = 0;
}


boolean Animatrix::getBrakeState(){
  return _brake_on;
}

void Animatrix::changeState(char state){
  switch(state) {
      case 'B':
        _brake_on = true;
        break;
      case 'b':
        _brake_on = false;
        break;
      case 'L':
        _current_slides = &LEFT_SLIDES[0][0];
        break;
      case 'R':
        _current_slides = &RIGHT_SLIDES[0][0];
        break;
      case 'C':
        _current_slides = &IDLE_SLIDES[0][0];
        break;
      default:
        Serial.print("ERROR: UNIDENTIFIED INPUT STATE: ");
        Serial.println(state);
        break;
    }
}

unsigned char * Animatrix::getNextSlide(){
  // First check and attenuate for valid slide number
  int num_vals = sizeof(*_current_slides);
  int num_slides = num_vals/49;
  Serial.print("num_vals = "); Serial.print(num_vals);
  Serial.print(" _slide_number = "); Serial.println(_slide_number);
  if(_slide_number >= num_slides)
    _slide_number = 0;
  
  // Load current slide
  int row_offset = _slide_number*7;
  
  // Update slide number
  _slide_number++;
  
  // Return array pointer
  return _current_slides+row_offset*7;
}// end loadNextSlide
