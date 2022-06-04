#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "logging.h"

#define MEM_SIZE (4096)
#define START_ADDR (0x200)
#define APP_SIZE (MEM_SIZE - START_ADDR)

#define VX(i) (((i) >> 8) & 0xf)
#define VY(i) (((i) >> 4) & 0xf)
#define NNN(i) ((i) & 0xfff)
#define KK(i) ((i) & 0xff)

typedef enum _Chip8Exception Chip8Exception;

enum _Chip8Exception {
  OK = 0,
  ILL,
};

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

static inline Chip8 *c8_init(Chip8 *self) {
  self->pc = 0x200;
  return self;
}

static inline Chip8 *c8_new() {
  return c8_init(malloc(sizeof(Chip8)));
}

void c8_load(Chip8 *self, uint8_t *app, int size) {
  assert(self);
  assert(app);
  assert(size > 0 && size < APP_SIZE);
  memcpy(self->mem, app, size);
}

static inline uint16_t c8_fetch(Chip8 *self) {
  assert(!(self->pc & 1));
  uint16_t i = *(uint16_t *)(self->mem + (self->pc - START_ADDR));
  self->pc += sizeof(uint16_t);
  return i;
}

static inline void c8_clear(Chip8 *self) {
}

static inline void c8_ret(Chip8 *self) {
}

static inline void c8_push_ip(Chip8 *self) {
}

static inline void c8_pop_ip(Chip8 *self) {
}

static inline void c8_jmp(Chip8 *self) {
}

static inline bool c8_key_pressed(Chip8 *self, int8_t key) {
  return false;
}

static inline int8_t c8_key_read(Chip8 *self) {
  return 0;
}

bool c8_step(Chip8 *self) {
  uint16_t instr = c8_fetch(self);
  switch(instr >> 12) {
    case 0x0:
      switch(instr & 0xfff) {
        case 0x0e0:
          c8_clear(self);
          break;
        case 0x0ee:
          c8_ret(self);
          break;
      }
      break;
    case 0x2:
      c8_push_ip(self);
    case 0x1:
      c8_jmp(self);
      break;
    case 0x3:
      if(NNN(instr) == self->v[VX(instr)]) {
        c8_fetch(self);
      }
      break;
    case 0x4:
      if(NNN(instr) != self->v[VX(instr)]) {
        c8_fetch(self);
      }
      break;
    case 0x5:
      if(self->v[VX(instr)] == self->v[VY(instr)]) {
        c8_fetch(self);
      }
      break;
    case 0x6:
      self->v[VX(instr)] = NNN(instr);
      break;
    case 0x7:
      self->v[VX(instr)] += NNN(instr);
      break;
    case 0x8:
      switch(instr & 0xf) {
        case 0x0:
          self->v[VX(instr)] = self->v[VY(instr)];
          break;
        case 0x1:
          self->v[VX(instr)] |= self->v[VY(instr)];
          break;
        case 0x2:
          self->v[VX(instr)] &= self->v[VY(instr)];
          break;
        case 0x3:
          self->v[VX(instr)] ^= self->v[VY(instr)];
          break;
        case 0x4:
          int r = self->v[VX(instr)] + self->v[VY(instr)];
          self->v[0xf] = (r > 0xff);
          self->v[VX(instr)] = r & 0xff;
          break;
        case 0x5:
          self->v[0xf] = (self->v[VX(instr)] > self->v[VY(instr)]);
          self->v[VX(instr)] -= self->v[VY(instr)];
          break;
        case 0x6:
          self->v[0xf] = self->v[VX(instr)] & 0x1;
          self->v[VX(instr)] >>= 1;
          break;
        case 0x7:
          self->v[0xf] = (self->v[VY(instr)] > self->v[VX(instr)]);
          self->v[VY(instr)] -= self->v[VX(instr)];
          break;
        case 0xe:
          self->v[0xf] = self->v[VX(instr)] >> 7;
          self->v[VX(instr)] <<= 1;
          break;
        default:
          assert(false);
      }
      break;
    case 0x9:
      if(self->v[VY(instr)] != self->v[VX(instr)]) {
        c8_fetch(self);
      }
      break;
    case 0xa:
      self->i = instr & 0xfff;
      break;
    case 0xb:
      self->pc = (NNN(instr) + self->v[0]) & 0xffff;
      break;
    case 0xc:
      char rnd = 0;
      self->v[VX(instr)] = KK(instr) & rnd;
      break;
    case 0xd:
      // TODO
      break;
    case 0xe:
      switch(instr & 0xff) {
        case 0x9e:
          if(c8_key_pressed(self, self->v[VX(instr)])) {
            c8_fetch(self);
          }
          break;
        case 0xa1:
          if(!c8_key_pressed(self, self->v[VX(instr)])) {
            c8_fetch(self);
          }
          break;
        default:
          assert(false);
      }
      break;
    case 0xf:
      switch(instr & 0xff) {
        case 0x07:
          self->v[VX(instr)] = self->dt;
          break;
        case 0x0a:
          self->v[VX(instr)] = c8_key_read(self);
          break;
        case 0x15:
          self->dt = self->v[VX(instr)];
          break;
        case 0x18:
          self->st = self->v[VX(instr)];
          break;
        case 0x1e:
          self->i += self->v[VX(instr)];
          break;
        case 0x29:
          // TODO
          break;
        case 0x33:
          // TODO
          break;
        case 0x55:
          memcpy(self->mem + self->i, self->v, VX(instr));
          break;
        case 0x65:
          memcpy(self->v, self->mem + self->i, VX(instr));
          break;
        default:
          assert(false);
      }
      break;
  }
  return false;
}

void c8_dump(Chip8 *self) {
  info("\n{ pc: %04d, i: %04d, dt: %03d, st: %03d, \n" \
       "  v0: %03d, v1: %03d, v2: %03d, v3: %03d, \n" \
       "  v4: %03d, v5: %03d, v6: %03d, v7: %03d, \n" \
       "  v8: %03d, v9: %03d, va: %03d, vb: %03d, \n" \
       "  vc: %03d, vd: %03d, ve: %03d, vf: %03d }",
       self->pc, self->i, self->dt, self->st,
       self->v[0x0], self->v[0x1], self->v[0x2], self->v[0x3],
       self->v[0x4], self->v[0x5], self->v[0x6], self->v[0x7],
       self->v[0x8], self->v[0x9], self->v[0xa], self->v[0xb],
       self->v[0xc], self->v[0xd], self->v[0xe], self->v[0xf]);
}

int main() {
  Chip8 *c8 = c8_new();
  assert(c8->pc == START_ADDR);
  while(c8_step(c8));
  assert(c8->pc == (START_ADDR + sizeof(int16_t)));
}
