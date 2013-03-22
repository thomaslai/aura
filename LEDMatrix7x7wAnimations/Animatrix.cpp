#include "Animatrix.h"

// Animatrix constructor
Animatrix::Animatrix(){
  _current_state = IDLE;
  _brake_on = false;
}

boolean Animatrix::getBrakeState(){
  return _brake_on;
}

char Animatrix::getCurrentState(){
  if(_current_state == IDLE)
    return 'I';
  else if(_current_state == LEFT)
    return 'L';
  else
    return 'R';
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
        _current_state = LEFT;
        break;
      case 'R':
        _current_state = RIGHT;
        break;
      case 'C':
        _current_state = IDLE;
        break;
    }
}
