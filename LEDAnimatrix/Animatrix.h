#ifndef Animatrix_h
#define Animatrix_h

#include <Arduino.h>


class Animatrix{
  public:
  // Variables
  
  // Functions
  Animatrix();
  boolean getBrakeState();
  void changeState(char state);
  unsigned char * getNextSlide();

  private:
  // Variables
  unsigned char * _current_slides;
  boolean _brake_on;
  int _slide_number;
  
  
  // Functions

}; // end class Animatrix
#endif
