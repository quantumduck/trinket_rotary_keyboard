    /*
    Using the TrinketKeyboard Vusb library to make a keyboard
    based on a rotary phone dial.
    */

    /*  Hardware:
     *  The TrinketKeyboard library is designed for the ATtiny
     *  microcontroller on the Teensey development board.
     *  Additional steps will be required to make this work
     *  on other Arduino boards.
     */

/*  Current Issues:

In file included from Arduino/libraries/TrinketKeyboard/usbdrvasm_includer.S:24:0:
Arduino/libraries/TrinketKeyboard/usbdrv/usbdrvasm.S:17:0: warning: "__SFR_OFFSET" redefined [enabled by default]

 #define __SFR_OFFSET 0      /* used by avr-libc's register definitions */

/*

In file included from c:\program files (x86)\arduino\hardware\tools\avr\avr\include\avr\io.h:99:0,

                 from C:\Users\Ben\Documents\Arduino\libraries\TrinketKeyboard\cmdline_defs.h:26,

                 from C:\Users\Ben\Documents\Arduino\libraries\TrinketKeyboard\usbdrvasm_includer.S:22:

c:\program files (x86)\arduino\hardware\tools\avr\avr\include\avr\sfr_defs.h:141:0: note: this is the location of the previous definition

 #    define __SFR_OFFSET 0x20
*/

    #include <TrinketKeyboard.h>

    #define TRUE  1
    #define FALSE 0
    #define LOWTOHIGH 3
    #define HIGHTOLOW 2
    #define ULMAX 4294967295L // largest unsigned long supported.
    #define TIMEOUTMAX 500000L // 0.5 seconds (see Timing: vars below)

    // Recommended HW pin usage:
    #define DIAL_PIN     0
    #define ALERT_PIN    1
    #define RECEIVER_PIN 2

    // Macros
    #define DIAL_RAW  (digitalRead(0))
    #define REC_RAW   (digitalRead(2))
    #define LED_ON    (digitalWrite(1, HIGH))
    #define LED_OFF   (digitalWrite(1, LOW))
    #define TIME_NOW  (micros())

    // Global State Variables:
    //// General:
    uint8_t dialing           = FALSE;
    uint8_t dialLevel         = HIGH;
    uint8_t lastDialLevel     = HIGH;
    uint8_t receiverLevel     = HIGH;
    uint8_t lastReceiverLevel = HIGH;
    int pulseCount            = 0;

    //// Timing:
    unsigned long elapsedTime(uint8_t pin);
    unsigned long now;
    unsigned long lastDialEvent;
    unsigned long lastRecEvent;
    unsigned long lastPoll;
    unsigned long dialDebounce = 5000L;
    unsigned long receiverDebounce = dialDebounce;
    unsigned long dialTimeout = 500000L;
    unsigned long debugLast = 0L;

    //// Debug:
    uint8_t debugLight        = LOW;
    uint8_t debugCount        = 0;
    void loopReturnDebug(int param);
    uint8_t debugLog[50];
    uint8_t debugLogIndex;
    //// Subroutine:
    uint8_t dialPoll();
    void debugLogAdd(uint8_t value);
    
    void setup()
    {
      // button pins as inputs
      pinMode(RECEIVER_PIN, INPUT);
      pinMode(DIAL_PIN, INPUT);
      pinMode(ALERT_PIN, OUTPUT);

      // setting input pins to high means turning on internal pull-up resistors
      digitalWrite(RECEIVER_PIN, HIGH);
      digitalWrite(DIAL_PIN, HIGH);
      // remember, the buttons are active-low, they read LOW when they are not pressed

      // start USB stuff
      TrinketKeyboard.begin();

      LED_ON;

      // Initialize the timers:
      debugLast = TIME_NOW;
      lastDialEvent = debugLast;
      debugLog[0] = dialPoll();
      debugLogIndex = 1;
    }

    void loop()
    {
      uint8_t poll_result;
      if (!dialing) {
        // The dial is not in use:
        if (pulseCount > 0) {
          // pulses have been recorded; print them out:
          for (int i=0; i<pulseCount; i++) {
            // this should type a capital A
            TrinketKeyboard.pressKey(KEYCODE_MOD_LEFT_SHIFT, KEYCODE_A);
            // this releases the key (otherwise it is held down!)
            TrinketKeyboard.pressKey(0, 0);
          }
          pulseCount = 0;
        } else {
          // no pulses are currently pending
          poll_result = dialPoll();
          debugLogAdd(poll_result);
          debugLogAdd(9);  
          if (poll_result == HIGHTOLOW) {
//            LED_OFF;
            dialing = TRUE;
          }
        }
      } else {
        // Dial is currently in use
        loopReturnDebug(0); // Print out state vars
        // Dialing Loop:
        poll_result = dialPoll();
        debugLogAdd(poll_result);
        if (poll_result == LOWTOHIGH) {
          pulseCount++;
        } else if (poll_result == HIGHTOLOW) {
          // Do nothing, for now.
        } else {
          // Steady state situation.
          if ((TIME_NOW - lastDialEvent) >= dialTimeout) {
            // Dial idle for a long time.
            if (dialLevel == HIGH) {
              // user likely not dialing
              dialing = FALSE;

            } else {
              // dial is active, but not moving.
            }
          } // else dial may still be active.
        }
      }
      // Continue

      /*
      if (digitalRead(PIN_BUTTON_CAPITAL_A) == LOW)
      {
        // this should type a capital A
        TrinketKeyboard.pressKey(KEYCODE_MOD_LEFT_SHIFT, KEYCODE_A);
        // this releases the key (otherwise it is held down!)
        TrinketKeyboard.pressKey(0, 0);
      }
     */

     /*
      if (digitalRead(PIN_BUTTON_STRING) == LOW)
      {
        // type out a string using the Print class
        TrinketKeyboard.print("Hello World!");
      }
     */
    }

    void loopReturnDebug(int param) {
      // Debug function that dumps current state variables as well as param
      // using the Vusb keyboard library. printing is triggered by the RECEIVER_PIN going low.
      // When running, it also
      if (REC_RAW == LOW) {
        TrinketKeyboard.print("Debug: ");
        TrinketKeyboard.print(param);
        TrinketKeyboard.print("; pulseCount = ");
        TrinketKeyboard.print(pulseCount);
        TrinketKeyboard.print("; dialing = ");
        TrinketKeyboard.print(dialing);
        TrinketKeyboard.print("; dialLevel = ");
        TrinketKeyboard.print(dialLevel);
        TrinketKeyboard.print("; lastDialLevel = ");
        TrinketKeyboard.print(lastDialLevel);
        TrinketKeyboard.print("; Log = ");
        for (int i=0; i<debugLogIndex; i++) {
          TrinketKeyboard.print(debugLog[debugLogIndex]); 
          TrinketKeyboard.print('-'); 
        }
        debugLog[0] = debugLog[debugLogIndex - 1];
        debugLogIndex = 1;
        debugLast = TIME_NOW;
        while ((TIME_NOW-debugLast) < dialTimeout ) {
          // wait
        }
      }
      if ((TIME_NOW - debugLast) >= dialTimeout) {
        debugLast = TIME_NOW;
        if (debugLight) {
          LED_OFF;
          debugLight = LOW;
        } else {
          LED_ON;
          debugLight = HIGH;
          debugCount++;
          if (debugCount > 5) {
            debugCount = 0;
            // Periodically print something:
            /*
            for (int i=0; i<pulseCount; i++) {
              // this should type a capital A
              TrinketKeyboard.pressKey(KEYCODE_MOD_LEFT_SHIFT, KEYCODE_B);
              // this releases the key (otherwise it is held down!)
              TrinketKeyboard.pressKey(0, 0);
            }
            */
            // periodically reset pulseCount.
            pulseCount = 0;

            /*
            if (dialing) {
              // this should type a capital A
              TrinketKeyboard.pressKey(KEYCODE_MOD_LEFT_SHIFT, KEYCODE_D);
              // this releases the key (otherwise it is held down!)
              TrinketKeyboard.pressKey(0, 0);
            } else {
              // this should type a capital A
              TrinketKeyboard.pressKey(0, KEYCODE_D);
              // this releases the key (otherwise it is held down!)
              TrinketKeyboard.pressKey(0, 0);
            }
            */
          }
        }
      }
    }

    unsigned long Elapsed_Time(uint8_t pin) {
      // Not yet implemented
    }

    uint8_t dialPoll() {
      // the poll function must be called at least once every 10 ms
      // or cause a keystroke
      // if it is not, then the computer may think that the device
      // has stopped working, and give errors
      TrinketKeyboard.poll();

      // Check if dial is now in use:
      dialLevel = DIAL_RAW;
      if (dialLevel != lastDialLevel) {
        // State change occurred.
        if ((TIME_NOW - lastDialEvent) >= dialDebounce) {
          // Persistant state change ocurred.
          lastDialLevel = dialLevel;
          lastDialEvent = TIME_NOW;
          if (dialLevel == LOW) {
            return HIGHTOLOW;
          } else {
            return LOWTOHIGH;
          }
        }
      }
      return dialLevel;
    }

    void debugLogAdd(uint8_t value) {
      if (value != debugLog[debugLogIndex - 1]) {
        if (debugLogIndex < 50) {
          debugLog[debugLogIndex] = value;
          debugLogIndex += 1;
        } else {
          debugLog[0] = value;
          debugLogIndex = 1;
        }
      }
    }
   
