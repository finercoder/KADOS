void clearBuffer(char* buffer[], int length);
int getNumSect(char fileName[]);
void printDirectory();
int mod(int a, int b);
void create(char fileName[]);

int main() {
	char input[513];
	char buffer[13312];
	char shell[8];
	char error[14];
	char err[6];
	char fileName[7];
	char storage[4];
	char current;
	int fileIndex;
	int file2;
	int sectNum;

	/* Set up buffer. */
	buffer[0] = '\0';

	err[0] = 'e';
	err[1] = 'r';
	err[2] = 'r';
	err[3] = '\n';
	err[4] = '\0';
	err[5] = '\0';

	/* Set up shell. */
	shell[0] = 'S';
	shell[1] = 'H';
	shell[2] = 'E';
	shell[3] = 'L';
	shell[4] = 'L';
	shell[5] = '>';
	shell[6] = ' ';
	shell[7] = '\0';

	/* Set up error. */
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


	/* Initialize terminating character to file name string */
	fileName[7] = '\0';

	sectNum = 0;

	while (1) {
		/*This puts SHELL> onto the terminal. */
		interrupt(0x21, 0, shell, 0, 0);

		/*This gets the input from the terminal. */
		interrupt(0x21, 1, input, 0, 0);

		/* type command. */
		if (input[0] == 't' && input[1] == 'y' && input[2] == 'p' && input[3] == 'e' && input[4] == ' ') {
			interrupt(0x21, 3, &(input[5]), buffer, 0);
			interrupt(0x21, 0, buffer, 0, 0);

			/* Pseudoflush buffer.s */
			buffer[0] = '\0';

		/* execute command. */
		} else if (input[0] == 'e' && input[1] == 'x' && input[2] == 'e' && input[3] == 'c' && input[4] == 'u' && input[5] == 't' && input[6] == 'e' && input[7] == ' ') {
			/* This tries to execute the program specified. */
			interrupt(0x21, 4, &(input[8]), 0x2000, 0);
		/* delete command */
		} else if (input[0] == 'd' && input[1] == 'e' && input[2] == 'l' && input[3] == 'e' && input[4] == 't' && input[5] == 'e' && input[6] == ' ') {
			interrupt(0x21, 7, &(input[7]), 0, 0);
		/* copy command */
		} else if (input[0] == 'c' && input[1] == 'o' && input[2] == 'p' && input[3] == 'y' && input[4] == ' ') {
			/* Get filename1 */
			for (fileIndex = 0; fileIndex < 6; fileIndex++) {
				current = input[5 + fileIndex];
				if (current == ' ') {
					break;
				}
				fileName[fileIndex] = current;
			}

			/* Get filename data */
			interrupt(0x21, 3, fileName, buffer, 0);

			/* Get sector number */
			sectNum = getNumSect(fileName);

			/* Get filename2 */
			file2 = 5 + fileIndex + 1;

			for (fileIndex = 0; fileIndex < 6; fileIndex++) {
				current = input[file2 + fileIndex];
				if (current == '\r') {
					break;
				}
				fileName[fileIndex] = current;
			}

			for (fileIndex = fileIndex; fileIndex < 6; fileIndex++) {
				fileName[fileIndex] = 0x00;
			}

			/* Copy */
			interrupt(0x21, 8, fileName, buffer, sectNum);

		} else if (input[0] == 'd' && input[1] == 'i' && input[2] == 'r' && input[3] == '\r') {
			printDirectory();
			/* no command found. */
		}	else if (input[0] == 'c' && input[1] == 'r' && input[2] == 'e' && input[3] == 'a' && input[4] == 't' && input[5] == 'e' && input[6] == ' ') {
			create(input + 7);
		} else {
			interrupt(0x21, 0, error, 0, 0);
		}
	}
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

void printDirectory() {
	char dir[513];
	char line[33];
	int sectNum;
	int dirIndex;
	int fileIndex;
	int lineIndex;

	/* fill directory */
	interrupt(0x21, 2, dir, 0x2, 0);

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

			while (lineIndex < 27) {
				line[lineIndex++] = ' ';
			}

			if (sectNum/10 != 0) {
				line[lineIndex++] = sectNum/10 + '0';
				line[lineIndex++] = mod(sectNum, 10) + '0';
			} else {
				line[lineIndex++] = ' ';
				line[lineIndex++] = sectNum +'0';
			}
			line[lineIndex++] = '\r';
			line[lineIndex++] = '\n';
			line[lineIndex] = 0x0;
			interrupt(0x21, 0, line, 0, 0);
		}
	}
}

int mod(int a, int b) {
  while (a >= b) {
    a = a - b;
  }

  return a;
}

void create(char fileName[]) {
	char prompt[9];
	char buffer[13312];
	char line[513];
	int bufferIndex;
	int lineIndex;

	prompt[0] = 'e';
	prompt[1] = 'n';
	prompt[2] = 't';
	prompt[3] = 'e';
	prompt[4] = 'r';
	prompt[5] = ':';
	prompt[6] = '\r';
	prompt[7] = '\n';
	prompt[8] = 0x0;

	bufferIndex = 0;

	interrupt(0x21, 0, prompt, 0, 0);

	while (bufferIndex < 13312) {
		interrupt(0x21, 1, line, 0, 0);

		if (line[0] == '\r') {
			break;
		}

		for (lineIndex = 0; lineIndex < 513; lineIndex++) {
			if (buffer[bufferIndex] != '\r') {
				break;
			}
			buffer[bufferIndex++] = line[lineIndex];
		}
	}
	buffer[bufferIndex] = 0x00;

	interrupt(0x21, 8, fileName, buffer, bufferIndex/512 + 1);

}
