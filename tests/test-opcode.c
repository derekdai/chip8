#include <stdio.h>
#include <assert.h>
#include "chip8.h"
#include "chip8-ops.h"
#include "logging.h"

int main() {
  AutoChip8 *vm = c8_new();
  assert(c8_pc(vm) == APP_ENTRY);
  uint8_t buf[4096];
  FILE *f = fopen("images/test_opcode.ch8", "r");
  assert(f != NULL);
  size_t n = fread(buf, 1, sizeof(buf), f);
  assert(n > 0);
  fclose(f);
  c8_load(vm, buf, n);
  c8_steps(vm, 200);
}
