# SMARTResponseXE_analyzer
Clear Channel Assessor for the SMARTResponseXE

This program uses the Clear Channel Assessment (CCA) mode of the 802.15.4 radio in the ATmega128RFA1 of the SMARTREsponseXE
It is set to CCA Mode 1 (Energy above threshold) and adjusted the threshold for each channel until it finds a signal. If none is found it moves on to the next channel. 

This project relies on the following repositories.

Board definitions come from the Sparkfun devel board (retired)

https://www.sparkfun.com/products/retired/11197

https://learn.sparkfun.com/tutorials/atmega128rfa1-dev-board-hookup-guide

Board Definitions File

https://cdn.sparkfun.com/assets/learn_tutorials/9/2/ATmega128RFA1_Addon.zip

Repo

https://github.com/sparkfun/ATmega128RFA1_Dev

Initial radio set up came from the BasicChat Sketch.

The text/drawing come from following repo

https://github.com/bitbank2/SmartResponseXE

To put it to sleep, press/hold the top left button. To wake up use the power button.
