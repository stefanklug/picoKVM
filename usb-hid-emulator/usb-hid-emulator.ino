/**************************************************************************
 *
 * File: CapsLockToggle.ino
 * Author: Julian Schuler (https://github.com/julianschuler)
 * License: GNU GPLv3, see LICENSE.txt
 * Description: This file is an example from the USBKeyboard library.
 *              It types in a message after hitting Caps Lock three times.
 *
 *************************************************************************/


#include "SerialCommand.h"
#include "KeyboardMouse.h"


SerialCommand SCmd;

int RXLED=17;




void process_keys_down()    
{ 
  char *arg; 

  uint8_t v[7] = {0};
  int i=0;

  while(i<7) {
	  arg = SCmd.next(); 
	  if(!arg) break;

	  v[i] = (uint8_t)atoi(arg);
	  i++;
  }

  KeyboardMouse.sendKeyboardReport(v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
}

void process_keys_up() {
	KeyboardMouse.sendKeyboardReport(0,0,0,0,0,0,0);
}

void process_keys() {
	process_keys_down();
	process_keys_up();
}

void process_mouse() {
	char *arg;
	uint8_t btn=0;
	int16_t x=0;
	int16_t y=0;
	int8_t wheel=0;

	arg = SCmd.next(); 
	if(arg) {
		btn = (uint8_t)atoi(arg);
		arg = SCmd.next();
	}

	if(arg) {
		x = (int16_t)atoi(arg);
		arg = SCmd.next();
	}

	if(arg) {
		y = (int16_t)atoi(arg);
		arg = SCmd.next();
	}

	if(arg) {
		wheel = (int8_t)atoi(arg);
		arg = SCmd.next();
	}

	/*Serial.print("M ");
	Serial.print(x);
	Serial.println(y);
*/

	KeyboardMouse.sendMouseReport( btn, x, y, wheel );
}

// This gets set as the default handler, and gets called when no other command matches. 
void unrecognized()
{
  Serial1.println("Unrecognized comand. Type H to get help."); 
}

void help()
{
  Serial1.println("Available commands:");
  Serial1.println("D m d0 [d1 d2 d3 d4 d5]  -- Sends Keyboard Report. Used to simulate KeyDown");
  Serial1.println("                            Sends Keyboard Report. m is the modifiers byte, d0-d5 are the key codes."); 
  Serial1.println("U                        -- Sends Zero Keyboard Report. Used to simulate key up on all keys");
  Serial1.println("K m d0 [d1 d2 d3 d4 d5]  -- Same as D ... folowed by U. Used to simulate a keystroke");
  Serial1.println("M btn x y                -- Sends Mouse Report. btn is the state of the buttons, x y are absolute mouse coordinates (0-32767) )."); 
}

void setup() {
	/* USB timing has to be exact, therefore deactivate Timer0 interrupt */
	//TIMSK0 = 0;
	Serial1.begin(57600);
	Serial1.println("HID Simulator");
	Serial1.println("Version 0.1");
	Serial1.println("==============================");
	help();

	//pinMode(RXLED, OUTPUT);

	//pinMode(14, INPUT_PULLUP);
/*USBCON = 0;
	UDIEN = 0;

        // Select endpoint 0.
        //savedUENUM = UENUM;
        UENUM = 0;

        // Disable endpoint 0 interrupts.
        //savedUEIENX0 = UEIENX;
        UEIENX = 0;*/

	SCmd.addCommand("D",process_keys_down);
	SCmd.addCommand("U",process_keys_up); 
	SCmd.addCommand("K",process_keys);
	SCmd.addCommand("M",process_mouse); 
	SCmd.addCommand("H",help); 
  	SCmd.addDefaultHandler(unrecognized);  // Handler for command that isn't matched  (says "What?") 

	//MyKeyboard.begin();
	//MyMouse.begin();
	//USB_INTR_ENABLE &= ~(1 << USB_INTR_ENABLE_BIT);
}


void loop() {
	static bool v = true;
	digitalWrite(RXLED, v);

	SCmd.readSerial();

	//delay(10);
}
