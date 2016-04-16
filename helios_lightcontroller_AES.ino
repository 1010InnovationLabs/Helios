/***********************************************************************
 *              ____        ______       ____        ______            *    
 *             /  " \      /    " \     /  " \      /    " \           *  
 *            /__|| |     // ____  \   /__|| |     // ____  \          *
 *               |: |    /  /    ) :)     |: |    /  /    ) :)         *
 *              _\  |   (: (____/ //     _\  |   (: (____/ //          * 
 *             /" \_|\   \        /     /" \_|\   \        /           *
 *            (_______)   \"_____/     (_______)   \"_____/            *
 *                                                                     *
 *            ___             __       _______     ________            *
 *           |"  |           /""\     |   _  "\   /"       )           *
 *           ||  |          /    \    (. |_)  :) (:   \___/            *
 *           |:  |         /' /\  \   |:     \/   \___  \              *
 *            \  |___     //  __'  \  (|  _  \\    __/  \\             *
 *           ( \_|:  \   /   /  \\  \ |: |_)  :)  /" \   :)            *
 *            \_______) (___/    \___)(_______/  (_______/             *
 *                                                                     *
 *        AUTHOR: Alec Selfridge                                       *
 *  DATE CREATED: Unknown                                              *
 * LAST REVISION: 04/05/16                                             *
 *       PROJECT: Helios                                               *
 *       VERSION: 0.5.0                                                *
 *   DESCRIPTION: Target device: ATTINY85 (Trinket 8MHZ/USBTinyISP)    *
 *           Four RGB LEDs, connected to three common pins (R, G, B),  *
 *           are configured by a push-button by cycling through a pre- *
 *           defined list. Said button is software debounced by        *
 *           comparing the time of the first low-to-high transition    * 
 *           with a later reading. If the input was HIGH in both       *
 *           cases, then we accept the input and cycle modes.          *
 *           This scheme applies to both buttons, each with different  *
 *           parameters they modify.                                   *
 *           V0.1: added CDS cell and non-blocking delays.             *
 *           V0.2: swapped NeoPixels for generic RGB LEDs.             *
 *           V0.3: removed CDS cell for a pattern selector.            *
 *                 one button cycles through speeds + colors.          *
 *           V0.4: removed OFF state from available modes. Changed     *
 *                 dbDelay to 100.                                     *
 *           V0.5: removed green blink patterns for prototype          *      
 *                                                                     *
 * No part of this document may be used, distributed, or reproduced    *
 * without written permission from 1010 Innovation Labs.               *
 ***********************************************************************/
void cycle(void);
void cyclePattern(void);
void setLights(void);
void debounceRead(int pin);

// defaults
#define ON  true
#define OFF false
#define DEFAULT_STATE {HIGH, LOW, LOW} // red

// maximum amount of supported modes (including 0)
#define MAX_MODES 4

// pin assignments
#define R_PIN      0
#define G_PIN      1
#define B_PIN      2
#define BTN_PIN    4

// state & status variables
unsigned char currentState = 0;
bool          lightState[] = DEFAULT_STATE;
bool          blinkStage   = OFF;
long          dbDelay      = 100;
int           btnState;
int           lastBtnState = LOW;
long          lastDbTime   = 0;
// light refresh variables
unsigned long T_rf     = 1000;     // refresh period for lights (ms)
unsigned long prevTime = millis(); // initialize the refresh clock
/*****************************
* Initializations
*****************************/
void setup() {
  pinMode(BTN_PIN, INPUT);
  // 'n' RGB LEDs can be connected in parallel to each pin
  // this allows synchronized control of many LEDs using only 3 pins
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
}
/*****************************
* Main
*****************************/
void loop() {
  /*****************************
   * User-input Handling
   *****************************/
  debounceRead(BTN_PIN);
  /*****************************
   * Light Refresh Handling
   *****************************/
  if((millis() - prevTime) > T_rf) {
    prevTime = millis(); blinkStage = !blinkStage;
    if(blinkStage) { // on cycle
      setLights();
      digitalWrite(R_PIN, lightState[0]);
      digitalWrite(G_PIN, lightState[1]);
      digitalWrite(B_PIN, lightState[2]);
    }
    else { // off cycle
      lightState[0] = LOW; lightState[1] = LOW; lightState[2] = LOW;
      digitalWrite(R_PIN, lightState[0]); 
      digitalWrite(G_PIN, lightState[1]); 
      digitalWrite(B_PIN, lightState[2]);
    }
  }
}

void cycle(void) {
  currentState++;
  if(currentState >= MAX_MODES)
    currentState = 0;
}

// variable-rate debouncer on any given pin. the rate is determined by dbDelay
void debounceRead(int pin) {
  int reading = digitalRead(pin);
  // if the button changed in any way, be it noise or a press...
  if (reading != lastBtnState) {
    // start the timer
    lastDbTime = millis();
  }
  // if the button has stayed HIGH longer than a threshold...
  if ((millis() - lastDbTime) > dbDelay) {
    // update the state of the button
    if (reading != btnState) {
      btnState = reading;
      // if the button stayed HIGH, then we assume it's a valid press
      if (btnState == HIGH) {
          cycle();
      }
    }
  }
  // this prevents one button press from being perceived as multiple presses
  lastBtnState = reading;
}

// sets the colors for each mode along with the blink period
void setLights(void) {
  switch(currentState) {
    // Off
    case 255: lightState[0] = LOW; lightState[1] = LOW; lightState[2] = LOW; T_rf = 500;
    break;
    // Red normal
    case 0: lightState[0] = HIGH; lightState[1] = LOW; lightState[2] = LOW; T_rf = 1000;
    break;
    // Red fast
    case 1: lightState[0] = HIGH; lightState[1] = LOW; lightState[2] = LOW; T_rf = 500;
    break;
    // Purple normal
    case 2: lightState[0] = HIGH; lightState[1] = LOW; lightState[2] = HIGH; T_rf = 1000;
    break;
    // Purple fast
    case 3: lightState[0] = HIGH; lightState[1] = LOW; lightState[2] = HIGH; T_rf = 500;
    break; 
    default: lightState[0] = HIGH; lightState[1] = HIGH; lightState[2] = HIGH;
    break;
  }
}
