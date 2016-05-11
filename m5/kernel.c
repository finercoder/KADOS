
void printString(char* string);
void readString(char stringArr[]);
void readSector(char* buffer, int sector);
int mod(int a, int b);
int div(int a, int b);
void handleInterrupt21(int ax, int bx, int cx, int dx);
void readFile(char stringArr[]);
int executeProgram(char* name);
void terminate();
void writeSector(char* buffer, int sector);
void deleteFile(char* fileName);
void writeFile(char* fileName, char* buffer, int numberSectors);
void handleTimerInterrupt(int segment, int sp);
void killProcess(int processID);
void stallShell(int processID);
void printProcessTable();
void clearTerminal();

/* Prints a given character */
void debugPrint(char printThis);

struct ProcessEntry {
  int isActive;
  int sp;
  int waiting; /* -1 no wait otherwise it is the processID of the process it waiting on.*/
};

struct ProcessEntry processTable[8];
int currentProcess;

#define PROCESS_TABLE_SIZE 8
#define SHELL_ID 0
#define SECTOR_SIZE 513
#define MAX_BUFFER_SIZE 13312
#define MAP_SECTOR 1
#define DIRECTORY_SECTOR 2

int main() {
  int i;
  char shell[6];
  char welcome[21];

  /* Set up shell string. */
  shell[0] = 's';
  shell[1] = 'h';
  shell[2] = 'e';
  shell[3] = 'l';
  shell[4] = 'l';
  shell[5] = '\0';

  /* Set up welcome message. */
  welcome[0] = 'W';
  welcome[1] = 'e';
  welcome[2] = 'l';
  welcome[3] = 'c';
  welcome[4] = 'o';
  welcome[5] = 'm';
  welcome[6] = 'e';
  welcome[7] = ' ';
  welcome[8] = 't';
  welcome[9] = 'o';
  welcome[10] = ' ';
  welcome[11] = 'K';
  welcome[12] = 'A';
  welcome[13] = 'D';
  welcome[14] = ' ';
  welcome[15] = 'O';
  welcome[16] = 'S';
  welcome[17] = '!';
  welcome[18] = '\r';
  welcome[19] = '\n';
  welcome[20] = '\0';

  /* Initialize global variables. */
  for (i = 0; i < PROCESS_TABLE_SIZE; i++) {
    processTable[i].isActive = 0;
    processTable[i].sp = 0xff00;
    processTable[i].waiting = -1;
  }
  currentProcess = -1;

  /* Set Interrupts. */
  makeInterrupt21();
  interrupt(0x21, 0, welcome, 0, 0);
  makeTimerInterrupt();


  /* Execute Shell. */
  interrupt(0x21, 4, shell, 0x2000, 0);

  /* Wait for interrupts. */
  while(1) {
  }
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

  while (index < SECTOR_SIZE) {
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
    executeProgram(bx);
  } else if (ax == 5) {
    terminate();
  } else if (ax == 6) {
    writeSector(bx, cx);
  } else if (ax == 7) {
    deleteFile(bx);
  } else if (ax == 8) {
    writeFile(bx, cx, dx);
  } else if (ax == 9) {
    killProcess(bx);
  } else if (ax == 10) {
    stallShell(executeProgram(bx));
  } else if (ax == 11) {
    printProcessTable();
  } else if (ax == 12) {
    clearTerminal();
  } else {
    printString(error);
  }
}

void readFile(char fileName[], char buffer[]) {
  char directory[SECTOR_SIZE];
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
  readSector(directory, DIRECTORY_SECTOR);

  /* Loop through the directory and find name. A line is 32 bytes */
  for (index = 0; index < SECTOR_SIZE; index = index + 32) {
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

int executeProgram(char* name) {
  char error[27];
  char maxProcs[24];
  char buffer[MAX_BUFFER_SIZE];
  int index;
  int processTableIndex;
  int segment;

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

  maxProcs[0] = 'E';
  maxProcs[1] = 'r';
  maxProcs[2] = 'r';
  maxProcs[3] = 'o';
  maxProcs[4] = 'r';
  maxProcs[5] = ':';
  maxProcs[6] = ' ';
  maxProcs[7] = 'M';
  maxProcs[8] = 'a';
  maxProcs[9] = 'x';
  maxProcs[10] = ' ';
  maxProcs[11] = 'p';
  maxProcs[12] = 'r';
  maxProcs[13] = 'o';
  maxProcs[14] = 'c';
  maxProcs[15] = 'e';
  maxProcs[16] = 's';
  maxProcs[17] = 's';
  maxProcs[18] = 'e';
  maxProcs[19] = 's';
  maxProcs[20] = '.';
  maxProcs[21] = '\r';
  maxProcs[22] = '\n';
  maxProcs[23] = '\0';

  /* Initalize array with 0. */
  for (index = 0; index < MAX_BUFFER_SIZE; index++) {
    buffer[index] = 0x00;
  }

  /* Reset index and Get file and set current. */
  index = 0;
  readFile(name, buffer);

  if (buffer[0] == '\0') {
    printString(error);
    return;
  }

  /* Find inactive and nonwaiting slot. */
  setKernelDataSegment();
  for (processTableIndex = 0; processTableIndex < PROCESS_TABLE_SIZE; processTableIndex++) {
    if (processTable[processTableIndex].isActive == 0 && processTable[processTableIndex].waiting == -1) {
      processTable[processTableIndex].sp = 0xff00;
      break;
    }
  }
  restoreDataSegment();

  /* Check that process limit has not been reached. */
  if (processTableIndex == PROCESS_TABLE_SIZE) {
    printString(maxProcs);
    return -1;
  }

  /* Put data into memory. */
  segment = (processTableIndex + 2) * 0x1000;
  for (index = 0; index < MAX_BUFFER_SIZE; index++) {
    putInMemory(segment, index, buffer[index]);
  }

  /* Initialize Registers */
  initializeProgram(segment);

  /* Set active bit and currentProcess global. */
  setKernelDataSegment();
  processTable[processTableIndex].isActive = 1;
  currentProcess = processTableIndex;
  restoreDataSegment();
  return processTableIndex;
}

void terminate() {
  setKernelDataSegment();
  processTable[currentProcess].isActive = 0;
  restoreDataSegment();
  while(1) {}
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
  char directory[SECTOR_SIZE];
  char map[SECTOR_SIZE];
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
  readSector(directory, DIRECTORY_SECTOR);
  readSector(map, MAP_SECTOR);

  for (index = 0; index < SECTOR_SIZE; index = index + 32) {
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
    map[sectors[index] - 1] = 0x00;
  }

  /* Write directory and map back to memory */
  writeSector(directory, DIRECTORY_SECTOR);
  writeSector(map, MAP_SECTOR);
}

void writeFile(char* fileName, char* buffer, int numberSectors) {
  char directory[SECTOR_SIZE];
  char map[SECTOR_SIZE];
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
  readSector(directory, DIRECTORY_SECTOR);
  readSector(map, MAP_SECTOR);

  while (dirIndex < SECTOR_SIZE) {
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
        for (mapIndex = 0; mapIndex < SECTOR_SIZE; mapIndex++) {
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
  writeSector(directory, DIRECTORY_SECTOR);
  writeSector(map, MAP_SECTOR);
}

void handleTimerInterrupt(int segment, int sp) {
  int i;
  int nextProcess;
  int nextSegment;
  int nextStackPointer;
  int currentWaitStatus;
  struct ProcessEntry isWaitingOn;

  setKernelDataSegment();
  nextSegment = segment;
  nextStackPointer = sp;
  nextProcess = div(segment, 0x1000) - 2;

  if (nextProcess >= 0) {
    processTable[nextProcess].sp = sp;
  }

  for (i = 1; i <= PROCESS_TABLE_SIZE; i++) {
    nextProcess = mod(currentProcess + i, PROCESS_TABLE_SIZE);
    currentWaitStatus = processTable[nextProcess].waiting;

    /* Is the current process waiting? */
    if (currentWaitStatus != -1) {
      isWaitingOn = processTable[currentWaitStatus];
      /* Is the thing I am waiting on "inactive" and not waiting on other things? */
      /* If so, I can run. Otherwise, move on */
      if (isWaitingOn.isActive == 0 && isWaitingOn.waiting == -1) {
        processTable[nextProcess].isActive = 1;
        processTable[nextProcess].waiting = -1;
      }
    }

    if (processTable[nextProcess].isActive == 1) {
      currentProcess = nextProcess;
      nextSegment = (currentProcess + 2) * 0x1000;
      nextStackPointer = processTable[currentProcess].sp;
      break;
    }
  }

  restoreDataSegment();
  returnFromTimer(nextSegment, nextStackPointer);
}

void killProcess(int processID) {
  if (processID < 0 || processID > 7) {
    return;
  }

  setKernelDataSegment();
  processTable[processID].isActive = 0;
  restoreDataSegment();
}

void stallShell(int processID) {
  setKernelDataSegment();
  processTable[SHELL_ID].waiting = processID;
  processTable[SHELL_ID].isActive = 0;
  restoreDataSegment();
}

void printProcessTable() {
  int processID;
  char processString[4];

  processID = 0;
  processString[1] = '\r';
  processString[2] = '\n';
  processString[3] = '\0';
  while (processID < 8) {
    setKernelDataSegment();
    if (processTable[processID].isActive) {
      restoreDataSegment();
      processString[0] = processID + '0';
      printString(processString);
      setKernelDataSegment();
    }
    restoreDataSegment();
    processID++;
  }
}

void clearTerminal() {
  int i;
  char blankLine[3];

  blankLine[0] = '\r';
  blankLine[1] = '\n';
  blankLine[2] = '\0';

  for (i = 0; i < 25; i++) {
    printString(blankLine);
  }
  interrupt(0x10, 0x200, 0, 0, 0);
  interrupt(0x10, 0x4200, 0, 0, 0);
}

/* ----------Utilities --------------------------*/

/*
  Print a single character for debugging.
*/
void debugPrint(char printThis) {
  char debugString[4];

  debugString[0] = printThis;
  debugString[1] = '\r';
  debugString[2] = '\n';
  debugString[3] = '\0';

  printString(debugString);
}

/*
  Modulus operator.
*/
int mod(int a, int b) {
  while (a >= b) {
    a = a - b;
  }

  return a;
}

/*
Integer Division.
*/
int div(int a, int b) {
  int quotient = 0;

  while ((quotient + 1) * b <= a) {
    quotient = quotient + 1;
  }

  return quotient;
}
