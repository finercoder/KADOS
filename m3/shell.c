int main() {
	char fileName[513];
	char buffer[13312];
	char shell[8];
	char error[21];

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
	error[0] = 'E';
	error[1] = 'r';
	error[2] = 'r';
	error[3] = 'o';
	error[4] = 'r';
	error[5] = ':';
	error[6] = ' ';
	error[7] = 'B';
	error[8] = 'a';
	error[9] = 'd';
	error[10] = ' ';
	error[11] = 'C';
	error[12] = 'o';
	error[13] = 'm';
	error[14] = 'm';
	error[15] = 'a';
	error[16] = 'n';
	error[17] = 'd';
	error[18] = '\r';
	error[19] = '\n';
	error[20] = '\0';

	while (1) {
		/*This puts SHELL> onto the terminal. */
		interrupt(0x21, 0, shell, 0, 0);

		/*This gets the input from the terminal. */
		interrupt(0x21, 1, fileName, 0, 0);

		/* type command. */
		if (fileName[0] == 't' && fileName[1] == 'y' && fileName[2] == 'p' && fileName[3] == 'e' && fileName[4] == ' ') {
			interrupt(0x21, 3, &(fileName[5]), buffer, 0);
			interrupt(0x21, 0, buffer, 0, 0);

			/* Pseudoflush buffer.s */
			buffer[0] = '\0';

		/* execute command. */
		} else if (fileName[0] == 'e' && fileName[1] == 'x' && fileName[2] == 'e' && fileName[3] == 'c' && fileName[4] == 'u' && fileName[5] == 't' && fileName[6] == 'e' && fileName[7] == ' ') {
			/* This tries to execute the program specified. */
			interrupt(0x21, 4, &(fileName[8]), 0x2000, 0);
		/* no command found. */
		} else {
			interrupt(0x21, 0, error, 0, 0);
		}
	}
}
