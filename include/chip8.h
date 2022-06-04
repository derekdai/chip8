#include <stdint.h>
#include <stdbool.h>

#pragma once

#define MEM_SIZE (4096)
#define START_ADDR (0x200)
#define APP_SIZE (MEM_SIZE - START_ADDR)

enum _Chip8Exception {
  OK = 0,
  ILL,
};

typedef enum _Chip8Exception Chip8Exception;

typedef struct _Chip8 Chip8;

struct _Chip8 {
  struct {
    uint16_t pc;
    uint16_t i;
    uint8_t dt;
    uint8_t st;
    union {
      int8_t v[16];
      struct {
        int8_t v0;
        int8_t v1;
        int8_t v2;
        int8_t v3;
        int8_t v4;
        int8_t v5;
        int8_t v6;
        int8_t v7;
        int8_t v8;
        int8_t v9;
        int8_t va;
        int8_t vb;
        int8_t vc;
        int8_t vd;
        int8_t ve;
        int8_t vf;
      };
    };
  };
  uint8_t stack[START_ADDR - 20];
  uint8_t mem[APP_SIZE];
};

Chip8 *c8_init(Chip8 *self);

Chip8 *c8_new();

void c8_load(Chip8 *self, uint8_t *app, int size);

bool c8_step(Chip8 *self);

void c8_dump(Chip8 *self);
