# Rotoary Dial Keyboard

This is a hardware project to use a dial from a rotary phone as a USB HID. 

## Microcontroller

The code is targeted at the ATtiny85 microcontroller. Specifically, the 
[Trinket](https://www.adafruit.com/product/1501) development board is used
because it has built in hardware for a USB connection. The chip itself does not
have native USB support, but [Adafruit](https://learn.adafruit.com/introducing-trinket/introduction)
has libraries for basic input devices.

## Arduino

The arduino IDE is used for the builds. This, along with the USB keyboard library
introduces a lot of overhead, but this is not a problem so far.

Some effort has been made to indicate what parts of the code are specific to the
hardware, so it should not be difficult to use a different microcontroller or board,
with a different USB library.

## Watch this space for more info