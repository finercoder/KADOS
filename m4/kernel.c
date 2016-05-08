void printString(char* string);
void readString(char stringArr[]);
void readSector(char* buffer, int sector);
int mod(int a, int b);
int div(int a, int b);
void handleInterrupt21(int ax, int bx, int cx, int dx);
void readFile(char stringArr[]);
void executeProgram(char* name, int segment);
void terminate();
void writeSector(char* buffer, int sector);
void deleteFile(char* fileName);
void writeFile(char* fileName, char* buffer, int numberSectors);

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

void printString(char string[]) {
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
  int ah;
  int cx;
  int dx;

  ah = 2;

  relativeSector = mod(sector, 18) + 1;
  head = mod(div(sector, 18), 2);
  track = div(sector, 36);

  cx = track * 256 + relativeSector;
  dx = head * 256;
  ax = ah * 256 + 1;
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
  } else if (ax == 6) {
    writeSector(bx, cx);
  } else if (ax == 7) {
    deleteFile(bx);
  } else if (ax == 8) {
    writeFile(bx, cx, dx);
  } else {
    printString(error);
  }
}

void readFile(char fileName[], char buffer[]) {
  char directory[513];
  int index;
  int strIndex;
  int isFound;
  int indexRead;
  int sectors[26];
  int sectorsLen;
  int strLen;

  /* Initalize variables */
  index = 0;
  isFound = 0;
  sectorsLen = 0;

  /* Get directory. */
  readSector(directory, 2);

  for (index = 0; index < 513; index = index + 32) {
    /* Reset strLen. */
    strLen = 0;

    /* Find matching file name in directory. */
    for (strIndex = 0; strIndex < 6; strIndex++) {
      if (fileName[strIndex] != directory[index + strIndex]) {
        break;
      }

      if (fileName[strIndex] == 0x00 || fileName[strIndex] == '\r' || (fileName[strIndex] == directory[index + strIndex] && strIndex == 5)) {
        isFound = 1;
        break;
      }
    }

    /* Found fileName. */
    if (isFound) {
      /* Get sectors. */
      for (indexRead = 6; indexRead < 32; indexRead++) {
        if (directory[index + indexRead] == 0x00) {
          break;
        }
        sectors[indexRead - 6] = directory[index + indexRead];
        sectorsLen++;
      }
      break;
    }
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
  for (index = 0; index < 13312; index++) {
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
  if (buffer[0] != '\0') {
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

void writeSector(char* buffer, int sector) {
  int relativeSector;
  int head;
  int track;
  int ax;
  int ah;
  int cx;
  int dx;

  ah = 3;

  relativeSector = mod(sector, 18) + 1;
  head = mod(div(sector, 18), 2);
  track = div(sector, 36);

  cx = track * 256 + relativeSector;
  dx = head * 256;
  ax = ah * 256 + 1;
  interrupt(0x13, ax, buffer, cx, dx);
}

void deleteFile(char* fileName) {
  char directory[513];
  char map[513];
  int index;
  int strIndex;
  int sectors[26];
  int sectorsLen;
  int isFound;
  int indexRead;

  /* Initalize Variables. */
  index = 0;
  isFound = 0;
  sectorsLen = 0;

  /* Read the directory and map. */
  readSector(directory, 2);
  readSector(map, 1);

  for (index = 0; index < 513; index = index + 32) {
    /* Find matching file name in directory. */
    for (strIndex = 0; strIndex < 6; strIndex++) {
      if (fileName[strIndex] != directory[index + strIndex]) {
        break;
      }

      if (fileName[strIndex] == '\0' || (fileName[strIndex] == directory[index + strIndex] && strIndex == 5)) {
        isFound = 1;
        break;
      }
    }

    /* Found fileName. */
    if (isFound) {

      /* Get sectors. */
      for (indexRead = 0; indexRead < 32; indexRead++) {
        if (directory[index + indexRead] == 0x00) {
          break;
        }

        sectors[indexRead] = directory[index + indexRead];
        sectorsLen++;
      }

      /* Set first byte of file name to 0x00 */
      directory[index] = 0x00;
      break;
    }
  }

  /* Clear map sectors */
  for (index = 0; index < sectorsLen; index++) {
    map[sectors[index] - 1] = 0x00; /* is this right? */
  }

  /* Write directory and map back to memory */
  writeSector(directory, 2);
  writeSector(map, 1);
}

void writeFile(char* fileName, char* buffer, int numberSectors) {
  char directory[513];
  char map[513];
  int dirIndex;
  int fileIndex;
  int mapIndex;
  int sectorWrite;
  int clearBlock;
  int clear;
  int isFound;

  /* Initalize Variables. */
  dirIndex = 0;
  mapIndex = 0;
  isFound = 0;

  /* Read directory and map.  */
  readSector(directory, 2);
  readSector(map, 1);

  while (dirIndex < 513) {
    /* Find an empty space in the directory */
    if (directory[dirIndex] == 0x00) {
      /* Write the first 6 characters of the filename */
      for (fileIndex = 0; fileIndex < 6; fileIndex++) {
        if (fileName[fileIndex] != '\0' && fileName[fileIndex] != '\r') {
          directory[dirIndex + fileIndex] = fileName[fileIndex];
        } else {
          directory[dirIndex + fileIndex] = 0x00;
        }
      }

      /* Update dirIndex */
      dirIndex = dirIndex + 6;

      /* Find sectors to write, add to the directory, and update map. */
      for (sectorWrite = 0; sectorWrite < numberSectors; sectorWrite++) {
        while (mapIndex < 513) {
          if (map[mapIndex] == 0x00) {
            /* Mark map as used and set directory to map value */
            map[mapIndex] = 0xFF;
            directory[dirIndex + sectorWrite] = mapIndex; /* is this right */

            /* Write the buffer to the sector and update buffer */
            writeSector(buffer, mapIndex);
            buffer = buffer + 512;

            /* Leave map loop */
            break;
          }
          mapIndex++;
        }
      }

      /* Update dirIndex */
      dirIndex = dirIndex + numberSectors;

      /* Clear the block */
      clearBlock = 32 - numberSectors - 6;
      for (clear = 0; clear < clearBlock; clear++) {
        directory[dirIndex + clear] = 0x00;
      }

      /* Leave main loop */
      break;
    }

    /* Failed to find open spot, continue looking */
    dirIndex = dirIndex + 32;
  }

  /* Write directory and map back to memory */
  writeSector(directory, 2);
  writeSector(map, 1);
}
