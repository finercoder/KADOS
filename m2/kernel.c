void printString(char* string);
void readString(char stringArr[]);
void readSector(char* buffer, int sector);
int mod(int a, int b);
int div(int a, int b);
void handleInterrupt21(int ax, int bx, int cx, int dx);

int main() {
  char line[80];

  char buffer[512];
  readSector(buffer, 30);
  printString(buffer);

  // makeInterrupt21();
  interrupt(0x21, 1, line, 0, 0);
  interrupt(0x21, 0, line, 0, 0);

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

  /* Set storage terminating character */
  storage[1] = '\0';

  while (index < 513) {
    /* Get input */
    stringArr[index] = interrupt(0x16, 0, 0, 0, 0);
    storage[0] = stringArr[index];

    /* Handles backspace when no input. */
    if (stringArr[index] == 0x8 && index == 0) {
      continue;
    }

    /* Print input and increment index. */
    printString(storage);
    index++;

    /* Handle backspace when there is input */
    if (stringArr[index - 1] == 0x8 && index >= 1) {
      stringArr[index - 2] = ' ';
      storage[0] = ' ';
      printString(storage);
      storage[0] = 0x8;
      printString(storage);
      index = index - 2;
    }

    /* Leave loop on enter key press */
    if (stringArr[index - 1] == 0xd) {
      break;
    }
  }

  /* Set up console */
  storage[0] = '\r';
  printString(storage);
  storage[0] = '\n';
  printString(storage);

  /* Wrap end of stringArr */
  stringArr[index++] = '\r';
  stringArr[index++] = '\n';
  stringArr[index] = '\0';
}

int mod(int a, int b) {
  while (a >= b) {
    a = a - b;
  }

  return a;
}

int div(int a, int b) {
  int quotient = 0;

  while ((quotient + 1) * b <= a) {
    quotient = quotient + 1;
  }

  return quotient;
}

void readSector(char* buffer, int sector) {
  int relativeSector;
  int head;
  int track;
  int ax;
  int cx;
  int dx;

  relativeSector = mod(sector, 18) + 1;
  head = mod(div(sector, 18), 2);
  track = div(sector, 36);

  cx = track * 256 + relativeSector;
  dx = head * 256;
  ax = 2 * 256 + 1;
  interrupt(0x13, ax, buffer, cx, dx);
}

void handleInterrupt21(int ax, int bx, int cx, int dx) {
  if (ax == 0) {
    printString(bx);
  } else if (ax == 1) {
    readString(bx);
  } else if (ax == 2) {
    readSector(bx, cx);
  } else {
    printString("Error\0");
  }
}
