/*
The MIT License (MIT)

Copyright (c) 2016-2017 Nick Potts

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "doorfunc.h"

/*triggerRelay triggers the relay for some time*/
void triggerRelay() {
  digitalWrite(RelayPin, RelaySet);
  delay(RelayDur);
  digitalWrite(RelayPin, RelayIdle);
}

/*updatePos updates the global pos variable*/
void updatePos() {
  pos = analogRead(A2DPin);
}

/*isClosed returns true if the door is in the 'closed' position*/
bool isClosed() { return percentOpen(pos) < 5; }

/*wideOpen returns true if the door is in the fully opened position*/
bool wideOpen() { return percentOpen(pos) > 95; }

/*isOpen returns true if the door is in any sort of 'open' position*/
bool isOpen() {
  return !isClosed();
}

/*same returns true if a and b are both within A2Djitter */
bool same(unsigned int a, unsigned int b) {
  return (a > b) ? (a - b < A2DJitter) : (b - a < A2DJitter);
}

/*DoorMotion returns the guessed Motion of the door: Up, Down, or stopped.
This takes ~300ms to perform, and assumes that if the A2D counts are not moving by more than 10 counts,
it isnt moving*/
Motion DoorMotion() {
  unsigned int then = pos; //capture value now
  delay(MotionWait);
  updatePos();
  bool increasing = (pos < then);
  if (same(then, pos)) return Stopped;
  if ( (PosIncreaseOpensDoor && increasing) || (!PosIncreaseOpensDoor && !increasing)) return MovingUp;
  return MovingDown;
}
/*This handy function takes some time periods and function pointers to attempt to put the door in some known position.
It loops up to duration numberof milliseconds, with a pause of at least loopdelay
*/
bool optimize(int loopdelay, unsigned long duration, bool (*exitCond)(void), void (*onstopped)(void), void (*onup)(void), void (*ondown)(void) ){
  unsigned long start = millis();
  while((unsigned long)(millis() - start) < duration) {
    updatePos();
    if (exitCond()) return true;
    switch(DoorMotion()) {
      case Stopped: // not moving
        onstopped();
        break;
       case MovingUp: //moving up
        onup();
        break;
       case MovingDown: //moving down
        ondown();
        break;
      default:
        return false;
    }
    delay(loopdelay); //wait for stuff to happen
  }
  return false;
}
bool openDoor() {
  //every 500ms, for 30000ms, exit if wideOpen, smashing the button if going down, or stopped
  return optimize(2000, 90000, &wideOpen, &triggerRelay, &updatePos, &updatePos);
}

bool closeDoor() {
  //every 500ms, for 30000ms, exit if isClosed(), smashing the button if going up, or stopped
  return optimize(2000, 90000, &isClosed, &triggerRelay, &updatePos, &updatePos);
}

/*this returns a number, from 0 to 100 that reperesents the open state of the door.
0 should be understood to be closed
25 should be understood to be 25% off the floor
50 should be understood to be halfway open
75 should be understood to be mostly open
100 should be understood to be fully open.

npos is the current ADC position of the door
*/
int percentOpen(unsigned int npos) {
  float f = 100;
  if (PosIncreaseOpensDoor) {
    if (npos >= DoorAtCeiling) return 100;
    if (npos <= DoorAtFloor) return 0;
    f *= (npos - DoorAtFloor);
    f /= (DoorAtCeiling - DoorAtFloor);
    return (unsigned int) f;
  } else {
    if (npos <= DoorAtCeiling) return 100;
    if (npos >= DoorAtFloor) return 0;
    f *= (DoorAtFloor - npos);
    f /= (DoorAtFloor - DoorAtCeiling);
  }
  return f;
}

unsigned int averagePosition(unsigned char powerOfTwoTimes ) {
  unsigned long long int summer;
  int max = 1 << powerOfTwoTimes;
  for(int i = 0; i<max; i++)
    summer += analogRead(A2DPin);
  return summer >> powerOfTwoTimes;
}


