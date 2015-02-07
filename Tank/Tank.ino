#include <avr/sleep.h>
#include <SoftwareSerial.h>

const bool  DEBUG = true;
const short LOOP_PAUSE = 200; // milliseconds
const short MAX_TIME_BEFORE_CHANGE = 20000 / LOOP_PAUSE;

// Drive outputs
// =============
const char PIN_MAIN_DRAIN_PUMP = 10;
const char PIN_MAIN_FILL_PUMP  = 11;
const char PIN_ERROR_LIGHT     = 13;

// Sensor inputs
// =============
const char PIN_FLOAT_FULL   = 0;
const char PIN_FLOAT_LOW    = 1;
const char PIN_FLOAT_EMPTY  = 2;

// User buttons
// =============
const char PIN_CYCLE_BUTTON = 2;


// Errors
const char ERROR_NONE             = 0;
const char ERROR_UNSAFE_LEVEL     = 1;
const char ERROR_OVERFLOW         = 2;
const char ERROR_UNDERFLOW        = 3;
const char ERROR_IMPOSSIBLE_STATE = 4;
const char ERROR_STATE_TIMEOUT    = 5;

// Mnemonic for status when debug printing
const char* STATE_TEXT = "IEF!";
const char* ERROR_TEXT[] = {
  "All's well",
  "Barrel isn't at a safe fill level",
  "Barrel overflowed while emptying the sump",
  "Barrel ran out of water for filling the sump",
  "Floats conflicting. The impossible has happened!",
  "Floats didn't change fast enough. Water leak? Pump failure?"
};

enum action_state {
  FILLING = 2,
  DRAINING = 1,
  RESTING = 0,
  ERRORED = 3
};

struct tank {
  action_state state;
  bool full = 0;
  bool low  = 0;
  bool empty = 0;
  
  char offset = 0;
  
  void readpins() {
    full  = digitalRead(PIN_FLOAT_FULL  + offset);
    low   = digitalRead(PIN_FLOAT_LOW   + offset);
    empty = digitalRead(PIN_FLOAT_EMPTY + offset);
  }
};
  
volatile bool cycle_button = 0;
char error;

struct tank sump;
struct tank barrel;
int state_timer;

void set_pumps(int state) {
  // These are both active low, so invert the logic
  digitalWrite(PIN_MAIN_DRAIN_PUMP, !(state == DRAINING));
  digitalWrite(PIN_MAIN_FILL_PUMP,  !(state == FILLING));
}

// This is an ISR for button presses
void set_button() {
  cycle_button = true;
}

bool get_button() {
  bool value = cycle_button;
  cycle_button = false;
  
  return value;
}

// Show the error on the Arduino's LED
void err(char code) {
  set_pumps(RESTING);
  
  error = code;
  sump.state = ERRORED;
  
  debug_print();
  Serial.println();
  Serial.write("Error: ");
  Serial.write(ERROR_TEXT[code]);
  Serial.println();
  
  for (char a = 0; a < code; a++) {
    digitalWrite(PIN_ERROR_LIGHT, 1);
    delay(200);
    digitalWrite(PIN_ERROR_LIGHT, 0);
    delay(100);
  }
  
  // Make up the rest of the loop
  delay(2000 - 300 * code);
}

// Prints a table showing the internal state of the system.
void debug_print()
{
  if (!DEBUG)
    return;
    
  Serial.write("      S:FLE:FLE\r\n");
  Serial.write("Data: ");
  Serial.write(STATE_TEXT[sump.state]);
  Serial.write(':');      
  Serial.write('0' + sump.full);
  Serial.write('0' + sump.low);
  Serial.write('0' + sump.empty);
  Serial.write(':');
  Serial.write('0' + barrel.full);
  Serial.write('0' + barrel.low);
  Serial.write('0' + barrel.empty);
  Serial.write(':');
  Serial.write('0' + error);
  Serial.write('0' + cycle_button);
  Serial.write(':');
  Serial.print(state_timer, HEX);
  Serial.write("\r\n"); 
}

void rock_state()
{
  // Read in our button state set by the ISR
  bool button = get_button();
  
  // Show everything's alright
  digitalWrite(PIN_ERROR_LIGHT, sump.state != ERRORED);
  
  // Emergency shutdown, this is an unsafe state
  if (barrel.empty || sump.empty) {
    err(ERROR_UNSAFE_LEVEL);
  }

  /* Disabled for testing. In the real world, enable these. They'll
     detect some possible sensor wonkiness. */
  if (!DEBUG) {
    if (sump.low && sump.full)
      err(ERROR_IMPOSSIBLE_STATE);
      
    if (barrel.empty && barrel.full)
      err(ERROR_IMPOSSIBLE_STATE); 
      
    if (barrel.low && barrel.full)
      err(ERROR_IMPOSSIBLE_STATE);     
  }
    
  switch(sump.state) {
    case RESTING:
      if (button) {
        sump.state = DRAINING;
        state_timer = MAX_TIME_BEFORE_CHANGE;  
      }
      
      break;
      
    case DRAINING:
      // When the sensor shows there's no more water, fill'r up
      if (sump.low) {
        sump.state = FILLING;
        
        // In FILLING, wait this many cycles to detect
        // a change in sump.low
        state_timer = MAX_TIME_BEFORE_CHANGE;
      }
      
      // Don't overflow the barrel
      if (barrel.full) {
        err(ERROR_OVERFLOW);
      }
      
      // Detect pump failure (hopefully). I figure if the full float
      // isn't changing, we should give up before there are problems.
      // The underflow of the timer variable doesn't really matter here
      // if it wasn't full before it shouldn't be full the next time.
      if (!state_timer-- && sump.full) {
        err(ERROR_STATE_TIMEOUT);
      }
      
      // Allow manual override
      if (button) {
        sump.state = FILLING;
        state_timer = MAX_TIME_BEFORE_CHANGE;
      }
      
      break;
      
    case FILLING:
      // Stop once sump is full
      if (sump.full) {
        sump.state = RESTING;
      }
      
      // Need to make sure we don't underflow barrel
      if (barrel.low) {
        err(ERROR_UNDERFLOW);
      }
      
      // Detect state counter = 0, and make sure that the sump
      // isn't still low (detects pump or line leaks)
      if (!state_timer-- && sump.low) {
        err(ERROR_STATE_TIMEOUT);
      }
      
      // Allow manual override
      if (button) {
        sump.state = RESTING;
      }
      
      break;
      
    case ERRORED:
      if (button) {
        sump.state = RESTING;
        error      = ERROR_NONE;
      } else {
        err(error);
      }
      
      break;
  }
}

void setup() {
  sump.state     = RESTING;
  sump.offset    = 4; // Start at digital pin 4
  barrel.offset  = 7; // Start at digital pin 7
  
  cycle_button = false;
  
  for (char a = 0; a < 10; a++)
    pinMode(a, INPUT_PULLUP);
    
  pinMode(PIN_MAIN_DRAIN_PUMP, OUTPUT);
  pinMode(PIN_MAIN_FILL_PUMP,  OUTPUT);
  pinMode(PIN_ERROR_LIGHT,     OUTPUT);
  
  attachInterrupt(0, set_button, FALLING);
  
  Serial.begin(115200);
}

void loop() {
  // Load up our inputs
  barrel.readpins();
  sump.readpins();

  // Change the sump's state based on outside influence
  rock_state();  

  // Show what's going down (only in debug mode)
  debug_print();

  // Take action on sump's state
  set_pumps(sump.state);
  
  delay(LOOP_PAUSE);
}
