#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "chip8.h"

#define AutoFile Auto(FILE, _fclose)

static void _fclose(FILE **f) {
  if(*f) { fclose(*f); *f = NULL; }
}

uint8_t buf[USER_SIZE];

int main(int argc, char *argv[]) {
  if(argc <= 1) {
    printf("Usage: %s FILE.ch8 [STEPS]\n" \
           "  FILE.ch8 Chip8 program to load\n" \
           "  STEPS number of opcodes to run\n",
           argv[0]);
    exit(1);
  }

  int64_t steps = 0;
  if(argc > 2) {
    steps = strtoull(argv[2], NULL, 10);
    if(errno) {
      printf("'%s' is not valid STEPS: %s\n", argv[2], strerror(errno));
      exit(1);
    }
  }

  AutoFile *f = fopen(argv[1], "r");
  if(!f) {
    printf("unable to open %s: %s",
           argv[1],
           strerror(errno));
    exit(1);
  }

  if(fseek(f, 0, SEEK_END)) {
    printf("unable to seek to the end of %s: %s",
           argv[1],
           strerror(errno));
    exit(1);
  }

  long size = ftell(f);
  if(size <= 0) {
    printf("unable to get size of file %s: %s",
           argv[1],
           strerror(errno));
    exit(1);
  }

  if(size > USER_SIZE) {
    printf("%s too large (<= %d) to load",
           argv[1],
           USER_SIZE);
    exit(1);
  }

  rewind(f);
  if(errno) {
    printf("unable to rewind file position: %s", strerror(errno));
    exit(1);
  }

  if(fread(buf, 1, sizeof(buf), f) != size) {
    printf("unable to load %s at once: %s", argv[1], strerror(errno));
    exit(1);
  }

  AutoChip8 *vm = c8_new();
  c8_load(vm, buf, size);
  if(steps) {
    c8_steps(vm, steps);
  } else {
    while(true) {
      c8_step(vm);
    }
  }
}
