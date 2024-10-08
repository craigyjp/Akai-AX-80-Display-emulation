# This project was created to emulate the circuitry and displays used in the Akai AX-80 Polysynth.

I recently purchased both the Akai AX-80 processor board and voice board and managed to get them working to create a polysynth. 

Unfortunately I didn't have the nice AX-80 displays of the 1980's, so I decided that I could decode the original address and data lines of the AX-80 with modern micros and drive OLED screens using the online schematics.

![Synth](photos/synth.jpg)

In my use case I was not restricted by the dimensions of the original AX-80 keyboard as this was to be built into a 19" rack unit. Therefore I used 2" SPI IPS displays with 65,000 colours. These are probably too tall and not wide enough to fit the original AX-80 keyboard, but I have found some 3.12" displays that would possibly fill the AX-80 display windows but they are mono and not colour, so it is a bit of a trade off.


