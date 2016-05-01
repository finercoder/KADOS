kernel.c
==============
- The kernel.c file makes an interrupt (interrupt 0x21) and starts our shell.
- It also defines such functions as printString(), readString(), mod(), div(), readSector(), and other utilities.

shell.c
==============
- The shell.c file loops infinitely, using interrupts to print a shell prompt and accepts string input.
- If the string input is a valid command (currently in the form of "type filename" or "execute filename") it makes an interrupt and performs the appropriate function on the file specified by "filename."

Testing
==============
- Verification that our OS works so far is done by running from our Makefile ("make run"). Test commands can be run from the terminal.
- For example, typing the command "type messag" into our shell loads the messag file and prints it.
- Typing the command "execute tstpr2" loads the tstpr2 (tstprg will hang) program onto shell's location on the disk and runs it. It should print a message and return to the shell.
