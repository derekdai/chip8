#include <stdio.h>
#include <assert.h>
#include "chip8.h"
#include "chip8-ops.h"
#include "logging.h"

int main() {
  AutoChip8 *vm = c8_new();
  assert(c8_pc(vm) == APP_ENTRY);
  c8_load(vm, (uint8_t[2]){OP_NOP}, 2);
  c8_step(vm);
  assert(c8_pc(vm) == (APP_ENTRY + sizeof(int16_t)));
}
