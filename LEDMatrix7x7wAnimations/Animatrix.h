#ifndef Animatrix_h
#define Animatrix_h

#include <Arduino.h>

class Animatrix{
  public:
  // Variables
  
  // Functions
  Animatrix();
  boolean getBrakeState();
  char getCurrentState();
  void changeState(char state);
  void nextSlide();

  private:
  // Variables
  enum SignalState {IDLE, LEFT, RIGHT};
  SignalState _current_state;
  boolean _brake_on;
  int _slide_number;
  
  // Functions

}; // end class Animatrix
#endif
