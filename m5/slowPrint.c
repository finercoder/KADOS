int mod(int a, int b);

int main() {
  int i;
  int j;
  char hi[5];

  hi[0] = 'h';
  hi[1] = 'i';
  hi[2] = '\r';
  hi[3] = '\n';
  hi[4] = '\0';

  enableInterrupts();

  for (i = 0; i < 10000; i++) {
    for (j = 0; j < 10000; j++) {
    }
    if (mod(i, 31) == 0) {
      interrupt(0x21, 0, hi, 0, 0);
    }
  }

  interrupt(0x21, 5, 0, 0, 0);
}

int mod(int a, int b) {
  while (a >= b) {
    a = a - b;
  }

  return a;
}
