#include <stdio.h>
#include <assert.h>
#include "chip8.h"
#include "logging.h"

int main() {
  AutoChip8 *vm = c8_new();
  assert(c8_pc(vm) == 0x200);
  c8_load(vm, (uint8_t[2]){0x00, 0x00}, 2);
  c8_step(vm);
  assert(c8_pc(vm) == (0x200 + sizeof(int16_t)));
}
