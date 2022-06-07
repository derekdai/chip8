#include <stdint.h>
#include <stdbool.h>
#include "utils.h"

#ifndef __CHIP8_H_
#define __CHIP8_H_

#define APP_ENTRY (0x200)

#define AutoChip8 Auto(Chip8, _c8_free)

typedef union _OpCode OpCode;

union _OpCode {
  struct {
    union {
      uint16_t nnn: 12;
      struct {
        union {
          uint8_t kk: 8;
          struct {
            uint8_t subcode: 4;
            uint8_t vy: 4;
          };
        };
        uint8_t vx: 4;
      };
    };
    unsigned int code: 4;
  };
  struct {
    uint8_t lower;
    uint8_t upper;
  };
  uint16_t raw;
};

typedef struct _Chip8 Chip8;

Chip8 *c8_new();

void c8_free(Chip8 *self);

static inline void _c8_free(Chip8 **p) { c8_free(*p); }

void c8_load(Chip8 *self, uint8_t *app, int size);

void c8_step(Chip8 *self);

void c8_steps(Chip8 *self, int steps);

void c8_dump(Chip8 *self);

int16_t c8_pc(Chip8 *self);

int8_t c8_sp(Chip8 *self);

int16_t c8_i(Chip8 *self);

int8_t c8_v(Chip8 *self, int v);

static inline int8_t c8_flag(Chip8 *self) { return c8_v(self, 0xf); }

int8_t c8_dt(Chip8 *self);

int8_t c8_st(Chip8 *self);

uint8_t c8_mem8(Chip8 *self, int addr);

uint16_t c8_mem16(Chip8 *self, int addr);

bool c8_stack_empty(Chip8 *self);

uint16_t c8_stack_peek(Chip8 *self);

#endif /* __CHIP8_H_ */
