void printString(char* string);

int main() {
  printString("Hello World\0");
  while(1) {
  }
}

void printString(char* string) {
  char current;
  int index;
  char al;
  char ah;
  int ax;

  index = 0;
  current = string[index];
  ah = 0xe;
  index++;

  while (current != '\0') {
    al = current;
    ax = ah * 256 + al;
    interrupt(0x10, ax, 0, 0, 0);
    current = string[index++];
  }
}
