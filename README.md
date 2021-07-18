# Inkkey-FW-for-Macro-Keypad-Pro-
a modified Firmware for the Macro Keypad Pro to use with inkkeys (https://there.oughta.be/a/macro-keyboard) like PC-Software

I'm lazy so i bought the very nice hardware from https://www.tindie.com/products/phoenixcnc/6-key-macro-keypad-with-rotary-encoder-and-display/ the build in software i also nice but not as it should be.

I found this hard and software where i like the software very much but am to lazy to build the hardware for it. So i tried to merge both things together. This is the repo for the arduino firmware to run on the hardware from hayri.

I adapted the code from t.o.b to use the 15 leds from the hardware and made it able to set the keycap color and display some text on the color display.

This is the first relase. There is space for improvement, so i would like to use icons on the display....

## Changes to the original FW from t.o.b

### Defaults

The key's are set to `KEY_F13`uo to `KEY_F18`. The leds are of and the keycaps have an ugly color.

### Additions to the serial protocol

#### Text display
`T 0 ff00ff 00ff00 ffffff`
Clears the display and sets textcolor to `ff00ff` textbackgroundcolor to `00ff00` and the displaybackground color to `ffffff`, the color is rgb in hex notation.

`T 1 i'm key one`
Sets the text a key `1` is the key number from 1-6. The text should not be longer the 10 characters

`T 7 The wheel`
Sets the text for wheel

#### Keycaps color
`K 1 ff00ff`
Sets the color a key `1` is the key number from 1-6, the color is rgb in hex notation.





