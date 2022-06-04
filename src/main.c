#include <stdio.h>
#include <assert.h>
#include "chip8.h"

int main() {
  Chip8 *c8 = c8_new();
  assert(c8->pc == START_ADDR);
  while(c8_step(c8));
  assert(c8->pc == (START_ADDR + sizeof(int16_t)));
}
