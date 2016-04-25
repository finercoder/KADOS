int main() {
	char fileName[513];
	
	while (1) {
		/*This puts SHELL> onto the terminal*/
		interrupt(0x21, 0, "SHELL> \0", 0, 0);

		/*This gets the input from the terminal*/
		interrupt(0x21, 1, fileName, 0, 0);

		/*This tries to execute the program specified*/
  		interrupt(0x21, 4, fileName, 0x2000, 0);
	}
}