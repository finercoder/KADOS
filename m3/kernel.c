void printString(char* string);
void readString(char stringArr[]);
void readSector(char* buffer, int sector);
int mod(int a, int b);
int div(int a, int b);
void handleInterrupt21(int ax, int bx, int cx, int dx);
void readFile(char stringArr[]);
void executeProgram(char* name, int segment);
void terminate();

int main() {
  /*
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
  */

  makeInterrupt21();
  interrupt(0x21, 4, "shell\0", 0x2000, 0);
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
  char error[16];

  /* Set up error string */
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
  error[11] = 'a';
  error[12] = 'x';
  error[13] = '\r';
  error[14] = '\n';
  error[15] = '\0';

  /* Case Statements */
  if (ax == 0) {
    printString(bx);
  } else if (ax == 1) {
    readString(bx);
  } else if (ax == 2) {
    readSector(bx, cx);
  } else if (ax == 3) {
    readFile(bx, cx);
  } else if (ax == 4) {
    executeProgram(bx, cx);
  } else if (ax == 5) {
    terminate();
  } else {
    printString(error);
  }
}

void readFile(char fileName[], char buffer[]) {
  char dir[513];
  int index;
  int strIndex;
  int isFound;
  int indexRead;
  int sectors[31];
  int sectorsLen;
  int strLen;

  /* Set variables */
  index = 0;
  isFound = 0;
  sectorsLen = 0;

  /* Get directory. */
  readSector(dir, 2);

  while (index < 513) {
    /* Reset strLen. */
    strLen = 0;

    /* Find matching file name in directory. */
    for (strIndex = 0; strIndex < 6; strIndex++) {
      if (fileName[strIndex] == dir[index + strIndex]) {
        strLen++;
      } else {
        break;
      }

      if (fileName[strIndex] == '\0' || (fileName[strIndex] == dir[index + strIndex] && strIndex == 5)) {
        isFound = 1;
        break;
      }
    }

    /* Found fileName. */
    if (isFound) {
      /* Get sectors. */
      for (indexRead = 0; indexRead < 32 - strLen; indexRead++) {
        if (dir[indexRead + index + strLen] == 0x00) {
          break;
        }
        sectors[indexRead] = dir[indexRead + index + strLen];
        sectorsLen++;
      }
      break;
    }

    /* Update index */
    index = index + 32;
  }

  /* Read sector */
  for (index = 0; index < sectorsLen; index++) {
    readSector(buffer, sectors[index]);
    buffer = buffer + 512;
  }
}

void executeProgram(char* name, int segment) {
  char buffer[13312];
  char error[27];
  int index;
  int isFound;
  char current;

  /* Set isFound */
  isFound = 0;

  /* Set up error string. */
  error[0] = 'E';
  error[1] = 'r';
  error[2] = 'r';
  error[3] = 'o';
  error[4] = 'r';
  error[5] = ':';
  error[6] = ' ';
  error[7] = 'C';
  error[8] = 'o';
  error[9] = 'm';
  error[10] = 'm';
  error[11] = 'a';
  error[12] = 'n';
  error[13] = 'd';
  error[14] = ' ';
  error[15] = 'n';
  error[16] = 'o';
  error[17] = 't';
  error[18] = ' ';
  error[19] = 'f';
  error[20] = 'o';
  error[21] = 'u';
  error[22] = 'n';
  error[23] = 'd';
  error[24] = '\r';
  error[25] = '\n';
  error[26] = '\0';

  /* Initalize array with 0. */
  for (index = 0; index < 1024; index++) {
    buffer[index] = 0x00;
  }

  /* Reset index and Get file and set current. */
  index = 0;
  readFile(name, buffer);
  current = buffer[index];

  /* Put data into memory. */
  while (index < 13312) {
    putInMemory(segment, index, current);
    current = buffer[++index];
  }

  /* Check if found. */
  if (buffer[0] != '\0') {
    isFound = 1;
  }

  /* If found launch, else terminate. */
  if (isFound) {
    launchProgram(segment);
  } else {
    interrupt(0x21, 0, error, 0, 0);
    terminate();
  }
}

void terminate() {
  char shell[6];

  /* Set up shell string. */
  shell[0] = 's';
  shell[1] = 'h';
  shell[2] = 'e';
  shell[3] = 'l';
  shell[4] = 'l';
  shell[5] = '\0';

  interrupt(0x21, 4, shell, 0x2000, 0);
}
