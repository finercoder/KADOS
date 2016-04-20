void printString(char* string);
void readString(char stringArr[]);

int main() {
  char line[80];
  int index;

  while(1) {
    printString("Enter a line: \0");
    readString(line);
    printString(line);
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

void readString(char stringArr[]) {
  int index;
  char storage[2];
  index = 0;

  storage[1] = '\0';

  while (index < 80) {
    stringArr[index] = interrupt(0x16, 0, 0, 0, 0);
    storage[0] = stringArr[index];
    printString(storage);
    index++;

    if (stringArr[index - 1] == 0xd) {
      break;
    }
  }

  storage[0] = '\r';
  printString(storage);
  storage[0] = '\n';
  printString(storage);

  stringArr[index++] = '\r';
  stringArr[index++] = '\n';
  stringArr[index] = '\0';
}
