// Fish tank water level sensor with internal pull-up resistor and detecting transitions and Debouncing without delay
const byte switchPin = 8;
byte oldSwitchState = HIGH;  // assume switch open because of pull-up resistor
const unsigned long debounceTime = 10;  // milliseconds
unsigned long switchPressTime;  // when the switch last changed state

void setup ()
  {
  Serial.begin (115200);
  pinMode (switchPin, INPUT_PULLUP);
  }  // end of setup

void loop ()
  {
  // see if switch is open or closed
  byte switchState = digitalRead (switchPin);

  // has it changed since last time?
  if (switchState != oldSwitchState)
    {
    // debounce
    if (millis () - switchPressTime >= debounceTime)
       {
       switchPressTime = millis ();  // when we closed the switch
       oldSwitchState =  switchState;  // remember for next time
       if (switchState == LOW)
          {
          Serial.println ("Switch closed.");
          }  // end if switchState is LOW
       else
          {
          Serial.println ("Switch opened.");
          }  // end if switchState is HIGH

       }  // end if debounce time up

    }  // end of state change

  // other code here ...

  }  // end of loop