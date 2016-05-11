
/* Helper methods */
int getNumSect(char fileName[]);
int mod(int a, int b);
void debugPrint(char printChar);

/* Command methods */
void type(char input[]);
void execute(char input[]);
void deleteFile(char input[]);
void copy(char inputPtr[]);
void printDirectory();
void create(char fileName[]);
void kill(char processIDChar[]);
void execForground(char input[]);
void error();
void ps();
void clearTerminal();
void bgColor(char color[], int offset);

int main() {
	char shell[8];
	char input[513];
	int i;

	/* Set up shell. */
	shell[0] = 'S';
	shell[1] = 'H';
	shell[2] = 'E';
	shell[3] = 'L';
	shell[4] = 'L';
	shell[5] = '>';
	shell[6] = ' ';
	shell[7] = '\0';

	enableInterrupts();

	/* Initialize the input array. */
	for (i = 0; i < 513; i++) {
		input[i] = '\0';
	}

	while (1) {
		/*This puts SHELL> onto the terminal. */
		interrupt(0x21, 0, shell, 0, 0);

		/*This gets the input from the terminal. */
		interrupt(0x21, 1, input, 0, 0);

		/* type command. */
		if (input[0] == 't' && input[1] == 'y' && input[2] == 'p' && input[3] == 'e' && input[4] == ' ') {
			type(input);

		/* execute command. */
		} else if (input[0] == 'e' && input[1] == 'x' && input[2] == 'e' && input[3] == 'c' && input[4] == 'u' && input[5] == 't' && input[6] == 'e' && input[7] == ' ') {
			execute(input);

		/* delete command */
		} else if (input[0] == 'd' && input[1] == 'e' && input[2] == 'l' && input[3] == 'e' && input[4] == 't' && input[5] == 'e' && input[6] == ' ') {
			deleteFile(input);
		/* copy command */
		} else if (input[0] == 'c' && input[1] == 'o' && input[2] == 'p' && input[3] == 'y' && input[4] == ' ') {
			copy(input);

		/* dir command */
		} else if (input[0] == 'd' && input[1] == 'i' && input[2] == 'r' && input[3] == '\r') {
			printDirectory();

		/* dir command, also bound to ls */
		} else if (input[0] == 'l' && input[1] == 's' && input[2] == '\r') {
			printDirectory();

		/* create command */
		}	else if (input[0] == 'c' && input[1] == 'r' && input[2] == 'e' && input[3] == 'a' && input[4] == 't' && input[5] == 'e' && input[6] == ' ') {
			create(&(input[7]));

		/* kill command */
		} else if (input[0] == 'k' && input[1] == 'i' && input[2] == 'l' && input[3] == 'l' && input[4] == ' ') {
			kill(&(input[5]));

		/* execForeground command */
		} else if (input[0] == 'e' && input[1] == 'x' && input[2] == 'e' && input[3] == 'c' && input[4] == 'f' && input[5] == 'o' && input[6] == 'r' && input[7] == 'e' && input[8] == 'g' && input[9] == 'r' && input[10] == 'o' && input[11] == 'u' && input[12] == 'n' && input[13] == 'd' && input[14] == ' ') {
			execForground(input);

		/* ps command */
		} else if (input[0] == 'p' && input[1] == 's' && input[2] == '\r') {
			ps();

		/* clearTerminal command */
		} else if (input[0] == 'c' && input[1] == 'l' && input[2] == 'e' && input[3] == 'a' && input[4] == 'r' && input[5] == '\r') {
			clearTerminal();

		/* bgColor command */
		} else if (input[0] == 'b' && input[1] == 'g' && input[2] == 'c' && input[3] == 'o' && input[4] == 'l' && input[5] == 'o' && input[6] == 'r' && input[7] == ' ') {
			bgColor(input, 4);

		/* bgColor command, also bound to bgc */
		} else if (input[0] == 'b' && input[1] == 'g' && input[2] == 'c' && input[3] == ' ') {
			bgColor(input, 0);

		/* no command found. */
		} else {
			error();
		}
	}
}

/*
	Prints the value of the given text file to the terminal.
*/
void type(char input[]) {
	char buffer[13312];
	char s1[4];
	int i;

	s1[1] = '\r';
	s1[2] = '\n';
	s1[3] = '\0';

	for (i  = 0; i < 13313; i++) {
		buffer[i] = '\0';
	}

	for (i = 5; i < 11; i++) {
		if (input[i] == '\r') {
			input[i] = '\0';
		}
	}

	input[11] = '\r';


	interrupt(0x21, 3, &(input[5]), buffer, 0);
	interrupt(0x21, 0, buffer, 0, 0);
}

/*
	Attempts to execute a specified program name.
*/
void execute(char input[]) {
	interrupt(0x21, 4, &(input[8]), 0x2000, 0);
}

/*
	Deletes a specified file.
*/
void deleteFile(char input[]) {
	char fileName[7];
	int i;

	/* Clear fileName. */
	for (i = 0; i < 7; i++) {
		fileName[i] = '\0';
	}

	/* Normalize input. */
	for (i = 7; i < 13; i++) {
		if (input[i] == '\r') {
			fileName[i - 7] = '\0';
		} else {
			fileName[i - 7] = input[i];
		}
	}

	/* Set carriage return character end. */
	fileName[6] = '\r';

	/* Delete file */
	interrupt(0x21, 7, fileName, 0, 0);
}

/*
	Copys a text file to a specified input.
*/
void copy(char input[]) {
	int fileIndex;
	int sectorNumbers;
	int fileOffset;
	int i;
	char data[13312];
	char fileName[7];
	char current;
	char duplicateError[40];
	duplicateError[0] = 'A';
	duplicateError[1] = ' ';
	duplicateError[2] = 'f';
	duplicateError[3] = 'i';
	duplicateError[4] = 'l';
	duplicateError[5] = 'e';
	duplicateError[6] = ' ';
	duplicateError[7] = 'w';
	duplicateError[8] = 'i';
	duplicateError[9] = 't';
	duplicateError[10] = 'h';
	duplicateError[11] = ' ';
	duplicateError[12] = 't';
	duplicateError[13] = 'h';
	duplicateError[14] = 'i';
	duplicateError[15] = 's';
	duplicateError[16] = ' ';
	duplicateError[17] = 'n';
	duplicateError[18] = 'a';
	duplicateError[19] = 'm';
	duplicateError[20] = 'e';
	duplicateError[21] = ' ';
	duplicateError[22] = 'a';
	duplicateError[23] = 'l';
	duplicateError[24] = 'r';
	duplicateError[25] = 'e';
	duplicateError[26] = 'a';
	duplicateError[27] = 'd';
	duplicateError[28] = 'y';
	duplicateError[29] = ' ';
	duplicateError[30] = 'e';
	duplicateError[31] = 'x';
	duplicateError[32] = 'i';
	duplicateError[33] = 's';
	duplicateError[34] = 't';
	duplicateError[35] = 's';
	duplicateError[36] = '.';
	duplicateError[37] = '\r';
	duplicateError[38] = '\n';
	duplicateError[39] = '\0';

	/* Initialize fileName and data */
	for (i = 0; i < 7; i++) {
		fileName[i] = '\0';
	}
	for (i = 0; i < 13313; i++) {
		data[i] = '\0';
	}

	/* Get filename1 */
	for (fileIndex = 0; fileIndex < 6; fileIndex++) {
		current = input[5 + fileIndex];
		if (current == ' ') {
			break;
		}
		fileName[fileIndex] = current;
	}

	/* Get filename data */
	interrupt(0x21, 3, fileName, data, 0);

	/* Get sector number */
	sectorNumbers = getNumSect(fileName);

	/* Get filename2 */
	fileOffset = 5 + fileIndex + 1;
	for (fileIndex = 0; fileIndex < 6; fileIndex++) {
		current = input[fileOffset + fileIndex];

		/* Leave loop early if return character detected */
		if (current == '\r') {
			break;
		}

		/* Set value of fileName (2) */
		fileName[fileIndex] = current;
	}

	/* Zero out the rest */
	for (fileIndex = fileIndex; fileIndex < 6; fileIndex++) {
		fileName[fileIndex] = 0x00;
	}

	/* get directory */

	if (isInDirectory(fileName)) {
		interrupt(0x21, 0, duplicateError, 0, 0);
		return;
	}


	/* Copy */
	interrupt(0x21, 8, fileName, data, sectorNumbers);
}

/*
	Prints the directory of the OS.
*/
void printDirectory() {
	char dir[513];
	char line[33];
	char heading[32];
	int sectNum;
	int dirIndex;
	int fileIndex;
	int lineIndex;

	/* fill directory */
	interrupt(0x21, 2, dir, 0x2, 0);

	/* initialize heading */
	heading[0] = 'F';
	heading[1] = 'i';
	heading[2] = 'l';
	heading[3] = 'e';
	heading[4] = ' ';
	heading[5] = 'n';
	heading[6] = 'a';
	heading[7] = 'm';
	heading[8] = 'e';
	heading[9] = ' ';
	heading[10] = ' ';
	heading[11] = ' ';
	heading[12] = ' ';
	heading[13] = ' ';
	heading[14] = ' ';
	heading[15] = ' ';
	heading[16] = ' ';
	heading[17] = ' ';
	heading[18] = 'N';
	heading[19] = 'o';
	heading[20] = '.';
	heading[21] = ' ';
	heading[22] = 's';
	heading[23] = 'e';
	heading[24] = 'c';
	heading[25] = 't';
	heading[26] = 'o';
	heading[27] = 'r';
	heading[28] = 's';
	heading[29] = '\r';
	heading[30] = '\n';
	heading[31] = '\0';

	/* print heading */
	interrupt(0x21, 0, heading, 0, 0);

	for (dirIndex = 0; dirIndex < 513; dirIndex = dirIndex + 32) {
		lineIndex = 0;
		if (dir[dirIndex] != 0x00) {
			for (fileIndex = 0; fileIndex < 6; fileIndex++) {
				if (dir[dirIndex + fileIndex] == 0x00) {
					break;
				}
				line[lineIndex++] = dir[dirIndex + fileIndex];
			}

			line[lineIndex] = 0x0;

			sectNum = getNumSect(line);

			/* Set spaces for alignment */
			while (lineIndex < 27) {
				line[lineIndex++] = ' ';
			}

			/* Get number of sectors */
			if (sectNum / 10 != 0) {
				line[lineIndex++] = sectNum / 10 + '0';
				line[lineIndex++] = mod(sectNum, 10) + '0';
			} else {
				line[lineIndex++] = ' ';
				line[lineIndex++] = sectNum +'0';
			}

			/* Set the ending characters. */
			line[lineIndex++] = '\r';
			line[lineIndex++] = '\n';
			line[lineIndex] = '\0';

			/* Print a line. */
			interrupt(0x21, 0, line, 0, 0);
		}
	}
}

/*
	Creates a text file. Request for each line until an empty line is entered.
*/
void create(char fileName[]) {
	char prompt[9];
	char buffer[13312];
	char line[513];
	int bufferIndex;
	int lineIndex;
	int i;
	int fileNameSize;

	/* Clear buffers. */
	for (i = 0; i < 13313; i++) {
		buffer[i] = '\0';
	}

	/* Normalize the filename with 0 buffers */
	for (fileNameSize = 0; fileNameSize < 6; fileNameSize++) {
		if (fileName[fileNameSize] == '\r') {
			fileName[fileNameSize] = '\0';
		}
	}

	/* Set last character to carriage return. */
	fileName[6] = '\r';

	/* Initialize prompt */
	prompt[0] = 'e';
	prompt[1] = 'n';
	prompt[2] = 't';
	prompt[3] = 'e';
	prompt[4] = 'r';
	prompt[5] = ':';
	prompt[6] = '\r';
	prompt[7] = '\n';
	prompt[8] = 0x0;

	/* Set bufferIndex */
	bufferIndex = 0;

	/* Print prompt */
	interrupt(0x21, 0, prompt, 0, 0);

	/* Get text to write */
	while (bufferIndex < 13313) {
		/* Clear line */
		for (i = 0; i < 513; i++) {
			line[i] = '\0';
		}

		/* Read line from terminal */
		interrupt(0x21, 1, line, 0, 0);

		/* Terminate once you reach the words at the end of the line */
		if (line[0] == '\r') {
			break;
		}

		/* Add line to buffer */
		for (lineIndex = 0; lineIndex < 513; lineIndex++) {
			if (line[lineIndex] == '\r') {
				break;
			}
			buffer[bufferIndex++] = line[lineIndex];
		}

		/* Set ending characters */
		buffer[bufferIndex++] = '\r';
		buffer[bufferIndex++] = '\n';
	}

	/* Null terminate the buffer */
	buffer[bufferIndex] = '\0';

	/* Write buffer */
	interrupt(0x21, 8, fileName, buffer, bufferIndex / 512 + 1);
}

/*
	Kill a process
*/
void kill(char processIDChar[]) {
	int processID;

	processID = processIDChar[0] - '0';

	interrupt(0x21, 9, processID, 0, 0);
}

void execForground(char input[]) {
	interrupt(0x21, 10, &(input[15]), 0x2000, 0);
}

/*
	Prints an error message.
*/
void error() {
	char error[14];

	error[0] = 'b';
	error[1] = 'a';
	error[2] = 'd';
	error[3] = ' ';
	error[4] = 'c';
	error[5] = 'o';
	error[6] = 'm';
	error[7] = 'm';
	error[8] = 'a';
	error[9] = 'n';
	error[10] = 'd';
	error[11] = '\r';
	error[12] = '\n';
	error[13] = '\0';

	interrupt(0x21, 0, error, 0, 0);
}

/* 
	Prints all running processes.
*/
void ps() {
	interrupt(0x21, 11, 0, 0, 0);
}

/* 
	Clears the screen, displaying only a shell prompt.
*/
void clearTerminal() {
	interrupt(0x21, 12, 0, 0, 0);
}

/*
	Changes the background color of the terminal.
*/
void bgColor(char color[], int offset) {
	int bx;
	char help[84];
	help[0] = 'a';
	help[1] = 'v';
	help[2] = 'a';
	help[3] = 'i';
	help[4] = 'l';
	help[5] = 'a';
	help[6] = 'b';
	help[7] = 'l';
	help[8] = 'e';
	help[9] = ' ';
	help[10] = 'c';
	help[11] = 'o';
	help[12] = 'l';
	help[13] = 'o';
	help[14] = 'r';
	help[15] = 's';
	help[16] = ':';
	help[17] = ' ';
	help[18] = 'b';
	help[19] = 'l';
	help[20] = 'a';
	help[21] = 'c';
	help[22] = 'k';
	help[23] = ',';
	help[24] = ' ';
	help[25] = 'b';
	help[26] = 'l';
	help[27] = 'u';
	help[28] = 'e';
	help[29] = ',';
	help[30] = ' ';
	help[31] = 'g';
	help[32] = 'r';
	help[33] = 'e';
	help[34] = 'e';
	help[35] = 'n';
	help[36] = ',';
	help[37] = ' ';
	help[38] = 'c';
	help[39] = 'y';
	help[40] = 'a';
	help[41] = 'n';
	help[42] = ',';
	help[43] = ' ';
	help[44] = 'r';
	help[45] = 'e';
	help[46] = 'd';
	help[47] = ',';
	help[48] = ' ';
	help[49] = 'p';
	help[50] = 'u';
	help[51] = 'r';
	help[52] = 'p';
	help[53] = 'l';
	help[54] = 'e';
	help[55] = ',';
	help[56] = ' ';
	help[57] = 'g';
	help[58] = 'r';
	help[59] = 'a';
	help[60] = 'y';
	help[61] = ',';
	help[62] = ' ';
	help[63] = 'l';
	help[64] = 'i';
	help[65] = 'm';
	help[66] = 'e';
	help[67] = ',';
	help[68] = ' ';
	help[69] = 't';
	help[70] = 'e';
	help[71] = 'a';
	help[72] = 'l';
	help[73] = ',';
	help[74] = ' ';
	help[75] = 'o';
	help[76] = 'r';
	help[77] = 'a';
	help[78] = 'n';
	help[79] = 'g';
	help[80] = 'e';
	help[81] = '\r';
	help[82] = '\n';
	help[83] = '\0';

	bx = 0;
	if (color[4 + offset] == 'b' && color[5 + offset] == 'l' && color[6 + offset] == 'a' && color[7 + offset] == 'c' && color[8 + offset] == 'k' && color[9 + offset] == '\r') {
		bx = 0x0;
	} else if (color[4 + offset] == 'b' && color[5 + offset] == 'l' && color[6 + offset] == 'u' && color[7 + offset] == 'e' && color[8 + offset] == '\r') {
		bx = 0x1;
	} else if (color[4 + offset] == 'g' && color[5 + offset] == 'r' && color[6 + offset] == 'e' && color[7 + offset] == 'e' && color[8 + offset] == 'n' && color[9 + offset] == '\r') {
		bx = 0x8;
	} else if (color[4 + offset] == 'c' && color[5 + offset] == 'y' && color[6 + offset] == 'a' && color[7 + offset] == 'n' && color[8 + offset] == '\r') {
		bx = 0x3;
	} else if (color[4 + offset] == 'r' && color[5 + offset] == 'e' && color[6 + offset] == 'd' && color[7 + offset] == '\r') {
		bx = 0x4;
	} else if (color[4 + offset] == 'p' && color[5 + offset] == 'u' && color[6 + offset] == 'r' && color[7 + offset] == 'p' && color[8 + offset] == 'l' && color[9 + offset] == 'e' && color[10 + offset] == '\r') {
		bx = 0x5;
	} else if (color[4 + offset] == 'g' && color[5 + offset] == 'r' && color[6 + offset] == 'a' && color[7 + offset] == 'y' && color[8 + offset] == '\r') {
		bx = 0x7;
	} else if (color[4 + offset] == 'l' && color[5 + offset] == 'i' && color[6 + offset] == 'm' && color[7 + offset] == 'e' && color[8 + offset] == '\r') {
		bx = 0xa;
	} else if (color[4 + offset] == 't' && color[5 + offset] == 'e' && color[6 + offset] == 'a' && color[7 + offset] == 'l' && color[8 + offset] == '\r') {
		bx = 0xb;
	} else if (color[4 + offset] == 'o' && color[5 + offset] == 'r' && color[6 + offset] == 'a' && color[7 + offset] == 'n' && color[8 + offset] == 'g' && color[9 + offset] == 'e' && color[10 + offset] == '\r') {
		bx = 0xc;
	} else {
		interrupt(0x21, 0, help, 0, 0);
		return;
	}
	interrupt(0x10, 0xb00, bx, 0, 0);
}

/*
	Prints a debug string with a given character.
*/
void debugPrint(char printChar) {
	char debugString[4];

	debugString[0] = printChar;
	debugString[1] = '\r';
	debugString[2] = '\n';
	debugString[3] = '\0';

	interrupt(0x21, 0, debugString, 0, 0);
}

int mod(int a, int b) {
  while (a >= b) {
    a = a - b;
  }

  return a;
}

int getNumSect(char fileName[]) {
	char dir[513];
	int sectNum;
	int dirIndex;
	int fileIndex;
	int isFound;

	sectNum = 0;
	isFound = 0;

	/* fill directory */
	interrupt(0x21, 2, dir, 0x2, 0);

	for (dirIndex = 0; dirIndex < 513; dirIndex = dirIndex + 32) {

		/* find first character of file name in directory*/
		if (dir[dirIndex] == fileName[0]) {
			for (fileIndex = 1; fileIndex < 7; fileIndex++) {
				if (fileName[fileIndex] == 0x00) {
					isFound = 1;
					break;
				} else if (fileName[fileIndex] != dir[dirIndex + fileIndex]) {
					break;
				}
			}

			/* once the file is found in the directory, find number of sectors */
			if (isFound) {
				for (fileIndex = 6; fileIndex < 32; fileIndex++) {
					if (dir[fileIndex + dirIndex] != 0x00) {
						sectNum++;
					} else {
						break;
					}
				}
				break;
			}
		}
	}
	return sectNum;
}

/* determines whether there is already a file in the directory with the given name */
int isInDirectory(char fileName[]) {
	char dir[513];
	int dirIndex;
	int fileIndex;
	int isFound;

	isFound = 0;

	/* fill directory */
	interrupt(0x21, 2, dir, 0x2, 0);

	for (dirIndex = 0; dirIndex < 513; dirIndex = dirIndex + 32) {

		/* find first character of file name in directory */
		if (dir[dirIndex] == fileName[0]) {
			for (fileIndex = 1; fileIndex < 7; fileIndex++) {
				if (fileName[fileIndex] == 0x00) {
					isFound = 1;
					break;
				} else if (fileName[fileIndex] != dir[dirIndex + fileIndex]) {
					break;
				}
			}
		}
	}
	return isFound;
}
