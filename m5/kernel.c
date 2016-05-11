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
void printLogo();

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

  printLogo();
  /* Set up shell string. */
  shell[0] = 's';
  shell[1] = 'h';
  shell[2] = 'e';
  shell[3] = 'l';
  shell[4] = 'l';
  shell[5] = '\0';

  /* Initialize global variables. */
  for (i = 0; i < PROCESS_TABLE_SIZE; i++) {
    processTable[i].isActive = 0;
    processTable[i].sp = 0xff00;
    processTable[i].waiting = -1;
  }
  currentProcess = -1;

  /* Set Interrupts. */
  makeInterrupt21();
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
  } else if (ax == 13) {

  } else if (ax == 14) {
    printLogo();
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

void printLogo() {
  char l1[32];
  char l2[32];
  char l3[32];
  char l4[32];

  l1[0] = ' ';
  l1[1] = ' ';
  l1[2] = '_';
  l1[3] = ' ';
  l1[4] = ' ';
  l1[5] = '_';
  l1[6] = '_';
  l1[7] = ' ';
  l1[8] = ' ';
  l1[9] = ' ';
  l1[10] = '_';
  l1[11] = ' ';
  l1[12] = ' ';
  l1[13] = ' ';
  l1[14] = '_';
  l1[15] = '_';
  l1[16] = '_';
  l1[17] = ' ';
  l1[18] = ' ';
  l1[19] = ' ';
  l1[20] = '_';
  l1[21] = '_';
  l1[22] = '_';
  l1[23] = ' ';
  l1[24] = ' ';
  l1[25] = '_';
  l1[26] = '_';
  l1[27] = '_';
  l1[28] = ' ';
  l1[29] = '\r';
  l1[30] = '\n';
  l1[31] = '\0';

  l2[0] = ' ';
  l2[1] = '|';
  l2[2] = ' ';
  l2[3] = '|';
  l2[4] = '/';
  l2[5] = ' ';
  l2[6] = '/';
  l2[7] = ' ';
  l2[8] = ' ';
  l2[9] = '/';
  l2[10] = '_';
  l2[11] = '\\';
  l2[12] = ' ';
  l2[13] = '|';
  l2[14] = ' ';
  l2[15] = ' ';
  l2[16] = ' ';
  l2[17] = '\\';
  l2[18] = ' ';
  l2[19] = '/';
  l2[20] = ' ';
  l2[21] = '_';
  l2[22] = ' ';
  l2[23] = '\\';
  l2[24] = '/';
  l2[25] = ' ';
  l2[26] = '_';
  l2[27] = '_';
  l2[28] = '|';
  l2[29] = '\r';
  l2[30] = '\n';
  l2[31] = '\0';

  l3[0] = ' ';
  l3[1] = '|';
  l3[2] = ' ';
  l3[3] = '\'';
  l3[4] = ' ';
  l3[5] = '<';
  l3[6] = ' ';
  l3[7] = ' ';
  l3[8] = '/';
  l3[9] = ' ';
  l3[10] = '_';
  l3[11] = ' ';
  l3[12] = '\\';
  l3[13] = '|';
  l3[14] = ' ';
  l3[15] = '|';
  l3[16] = ')';
  l3[17] = ' ';
  l3[18] = '|';
  l3[19] = ' ';
  l3[20] = '(';
  l3[21] = '_';
  l3[22] = ')';
  l3[23] = ' ';
  l3[24] = '\\';
  l3[25] = '_';
  l3[26] = '_';
  l3[27] = ' ';
  l3[28] = '\\';
  l3[29] = '\0';
  l3[29] = '\r';
  l3[30] = '\n';
  l3[31] = '\0';

  l4[0] = ' ';
  l4[1] = '|';
  l4[2] = '_';
  l4[3] = '|';
  l4[4] = '\\';
  l4[5] = '_';
  l4[6] = '\\';
  l4[7] = '/';
  l4[8] = '_';
  l4[9] = '/';
  l4[10] = ' ';
  l4[11] = '\\';
  l4[12] = '_';
  l4[13] = '\\';
  l4[14] = '_';
  l4[15] = '_';
  l4[16] = '_';
  l4[17] = '/';
  l4[18] = ' ';
  l4[19] = '\\';
  l4[20] = '_';
  l4[21] = '_';
  l4[22] = '_';
  l4[23] = '/';
  l4[24] = '|';
  l4[25] = '_';
  l4[26] = '_';
  l4[27] = '_';
  l4[28] = '/';
  l4[29] = '\r';
  l4[30] = '\n';
  l4[31] = '\0';

  printString(l1);
  printString(l2);
  printString(l3);
  printString(l4);
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
