#include <stdint.h>
#include <stdbool.h>
#include "utils.h"

#pragma once

enum _Chip8Exception {
  OK = 0,
  ILL,
};

typedef enum _Chip8Exception Chip8Exception;

#define AutoChip8 Auto(Chip8, _c8_free)

typedef struct _Chip8 Chip8;

Chip8 *c8_new();

void c8_free(Chip8 *self);

static inline void _c8_free(Chip8 **p) { c8_free(*p); }

void c8_load(Chip8 *self, uint8_t *app, int size);

bool c8_step(Chip8 *self);

// for testing/debugging purposes
void c8_dump(Chip8 *self);

int16_t c8_pc(Chip8 *self);

int8_t c8_sp(Chip8 *self);

int16_t c8_i(Chip8 *self);

int8_t c8_v(Chip8 *self, int v);

int8_t c8_dt(Chip8 *self);

int8_t c8_st(Chip8 *self);
