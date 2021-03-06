#include <stdio.h>
#include <assert.h>
#include "chip8.h"
#include "chip8-ops.h"
#include "logging.h"

int main() {
  AutoChip8 *vm = c8_new();
  assert(c8_pc(vm) == APP_ENTRY);
  c8_load(vm, (uint8_t[2]){OP_1nnn(0x200)}, 2);
  while(true) {
    c8_step(vm);
  }
}
