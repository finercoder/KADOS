The kernel.c file makes an interrupt (interrupt 0x21) and starts our shell.
It also defines such functions as printString(), readString(), mod(), div(), readSector(), and other utilities.

The shell.c file loops infinitely, using interrupts to print a shell prompt and accepts string input.
If the string input is a valid command (currently in the form of "type filename" or "execute filename") it makes
an interrupt and performs the appropriate function on the file specified by "filename." 

Verification that our OS works so far is done by running our Makefile and booting. Test commands can be run
from the terminal.