kernel.c
==============
- The kernel.c file makes an interrupt (interrupt 0x21) and starts our shell in its own process.
- It can run up to 8 process at a time through a scheduler using round robin
- It also defines such functions as printString(), readString(), mod(), div(), readSector(), and other utilities.

shell.c
==============
- The shell.c file loops infinitely, using interrupts to print a shell prompt and accepts string input.
- If the string input is a valid command (currently in the form of "type filename" or "execute filename") it makes an interrupt and performs the appropriate function on the file specified by "filename" in a new process.
- We have added the following special features to our shell:
  - you can change the background color of the terminal with "bgcolor" then a color (black, blue, cyan, red, purple, gray, lime, teal, orange); at startup you default to black. WARNING: SETTING COLOR TO GRAY IS NOT ADVISED SINCE THE TEXT IS GRAY
  - you can clear the terminal screen by typing "clear"
  - you can enter "ps" to display the pids that are currently active
  - when you perform a "dir" the shell prints out the names of all programs in the directory as well as the amount of sectors they each take up with column labels

Testing
==============
- Verification that our OS works so far is done by running from our Makefile ("make run"). Test commands can be run from the terminal.
- For example, typing the command "type messag" into our shell loads the messag file and prints it.
- Typing the command "execute tstpr2" loads the tstpr2 (tstprg will hang) program onto shell's location on the disk and runs it. It should print a message and return to the shell.
