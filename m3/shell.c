int main() {
	char fileName[513];
	char shell[8];

	shell[0] = 'S';
	shell[1] = 'H';
	shell[2] = 'E';
	shell[3] = 'L';
	shell[4] = 'L';
	shell[5] = '>';
	shell[6] = ' ';
	shell[7] = '\0';

	while (1) {
		/*This puts SHELL> onto the terminal*/
		interrupt(0x21, 0, shell, 0, 0);

		/*This gets the input from the terminal*/
		interrupt(0x21, 1, fileName, 0, 0);

		/*This tries to execute the program specified*/
  	interrupt(0x21, 4, fileName, 0x2000, 0);
	}
}
