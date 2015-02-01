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
const char PIN_CYCLE_BUTTON = 8;


// Errors
const char ERROR_UNSAFE_LEVEL     = 1;
const char ERROR_OVERFLOW         = 2;
const char ERROR_UNDERFLOW        = 3;
const char ERROR_IMPOSSIBLE_STATE = 4;
const char ERROR_STATE_TIMEOUT    = 5;

enum action_state {
  FILLING = 2,
  DRAINING = 1,
  RESTING = 0
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
  
char cycle_button = 0; // Not used at the moment

struct tank sump;
struct tank barrel;
int state_timer;

void set_pumps(int state) {
  // These are both active low, so invert the logic
  digitalWrite(PIN_MAIN_DRAIN_PUMP, !(state == DRAINING));
  digitalWrite(PIN_MAIN_FILL_PUMP,  !(state == FILLING));
}

// Show the error on the Arduino's LED
void err(char code) {
  set_pumps(RESTING);
  
  while (true) {
    for (char a = 0; a < code; a++) {
      digitalWrite(PIN_ERROR_LIGHT, 1);
      delay(200);
      digitalWrite(PIN_ERROR_LIGHT, 0);
      delay(100);
    }
    
    // Make up the rest of the loop
    delay(2000 - 300 * code);
  }
}

// Prints a table showing the internal state of the system
void debug_print()
{
  if (!DEBUG)
    return;
    
  if (Serial.available()) {
    Serial.write("      S:FLE:FLE\n");
    Serial.write("Data: ");
    Serial.write('0' + sump.state);
    Serial.write(':');      
    Serial.write('0' + sump.full);
    Serial.write('0' + sump.low);
    Serial.write('0' + sump.empty);
    Serial.write(':');
    Serial.write('0' + barrel.full);
    Serial.write('0' + barrel.low);
    Serial.write('0' + barrel.empty);
    Serial.write(':');
    Serial.print(state_timer, HEX);
    Serial.write("\n");
  }  
}

void rock_state()
{
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
      if (cycle_button) {
       sump.state = DRAINING; 
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
      if (!state_timer-- && sump.full) {
        err(ERROR_STATE_TIMEOUT);
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
      
      break;
  }
}

void go_sleep()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
}

void setup() {
  sump.state     = RESTING;
  sump.offset    = 2; // Start at digital pin 2
  barrel.offset  = 5; // Start at digital pin 5
  
  // Might as well start draining
  sump.state = DRAINING;
  state_timer = MAX_TIME_BEFORE_CHANGE;  
  
  for (char a = 0; a < 10; a++)
    pinMode(a, INPUT_PULLUP);
    
  pinMode(PIN_MAIN_DRAIN_PUMP, OUTPUT);
  pinMode(PIN_MAIN_FILL_PUMP,  OUTPUT);
  pinMode(PIN_ERROR_LIGHT,     OUTPUT);
  
  Serial.begin(9600);
}

void loop() {
  // Load up our inputs
  barrel.readpins();
  sump.readpins();

  // Change the sump's state based on outside influence
  rock_state();  

  // Take action on sump's state
  set_pumps(sump.state);      
  
  // Put the microcontroller to rest if it's done
  if (sump.state == RESTING) {
    go_sleep();
  }
  
  debug_print();
  delay(LOOP_PAUSE);
}
