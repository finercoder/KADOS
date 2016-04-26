void clearBuffer(char* buffer[], int length);

int main() {
	char input[513];
	char buffer[13312];
	char shell[8];
	char errorBadCommand[21];

	/* Set up buffer. */
	buffer[0] = '\0';

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
	errorBadCommand[0] = 'E';
	errorBadCommand[1] = 'r';
	errorBadCommand[2] = 'r';
	errorBadCommand[3] = 'o';
	errorBadCommand[4] = 'r';
	errorBadCommand[5] = ':';
	errorBadCommand[6] = ' ';
	errorBadCommand[7] = 'B';
	errorBadCommand[8] = 'a';
	errorBadCommand[9] = 'd';
	errorBadCommand[10] = ' ';
	errorBadCommand[11] = 'C';
	errorBadCommand[12] = 'o';
	errorBadCommand[13] = 'm';
	errorBadCommand[14] = 'm';
	errorBadCommand[15] = 'a';
	errorBadCommand[16] = 'n';
	errorBadCommand[17] = 'd';
	errorBadCommand[18] = '\r';
	errorBadCommand[19] = '\n';
	errorBadCommand[20] = '\0';

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
		/* no command found. */
		} else {
			interrupt(0x21, 0, errorBadCommand, 0, 0);
		}
	}
}
