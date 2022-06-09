#include <stdint.h>
#include <stdbool.h>
#include "utils.h"

#ifndef __CHIP8_H_
#define __CHIP8_H_

typedef uint16_t OpCode;

#define MEM_SIZE (0x1000)
#define VM_SIZE (0x200)
#define STACK_SIZE (0x60)
#define FRAMEBUFFER_SIZE (0x100)
#define USER_SIZE (MEM_SIZE - VM_SIZE - STACK_SIZE - FRAMEBUFFER_SIZE)

#define APP_ENTRY (0x200)

#define AutoChip8 Auto(Chip8, _c8_free)

typedef enum _Chip8Key Chip8Key;

enum _Chip8Key {
  C8_KEY_NIL = -1,
  C8_KEY_0 = 0,
  C8_KEY_1,
  C8_KEY_2,
  C8_KEY_3,
  C8_KEY_4,
  C8_KEY_5,
  C8_KEY_6,
  C8_KEY_7,
  C8_KEY_8,
  C8_KEY_9,
  C8_KEY_A,
  C8_KEY_B,
  C8_KEY_C,
  C8_KEY_D,
  C8_KEY_E,
  C8_KEY_F,
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

int8_t c8_v(Chip8 *self, uint8_t v);

static inline int8_t c8_flag(Chip8 *self) { return c8_v(self, 0xf); }

int8_t c8_dt(Chip8 *self);

int8_t c8_st(Chip8 *self);

uint8_t c8_mem8(Chip8 *self, int addr);

uint16_t c8_mem16(Chip8 *self, int addr);

bool c8_stack_empty(Chip8 *self);

uint16_t c8_stack_peek(Chip8 *self);

#endif /* __CHIP8_H_ */
