#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include "config.h"
#include "logging.h"
#include "chip8.h"
#include "ui.h"

#define MEM_SIZE (0x1000)
#define VM_SIZE (0x200)
#define STACK_SIZE (0x60)
#define FRAMEBUFFER_SIZE (0x100)
#define USER_SIZE (MEM_SIZE - VM_SIZE - STACK_SIZE - FRAMEBUFFER_SIZE)

#define VX(i) (((i) >> 8) & 0xf)
#define VY(i) (((i) >> 4) & 0xf)
#define NNN(i) ((i) & 0xfff)
#define KK(i) ((i) & 0xff)

struct _Chip8 {
  Ui *ui;

  // app 不可見/直接操作的 registers
  uint16_t pc;
  uint16_t sp;

  uint16_t i;
  uint8_t dt;
  uint8_t st;
  uint8_t v[16];

  union {
    uint8_t mem[MEM_SIZE];
    struct {
      uint8_t sys[VM_SIZE];
      uint8_t user[USER_SIZE];
      uint8_t fb[FRAMEBUFFER_SIZE];
      uint8_t stack[STACK_SIZE];
    };
  };
};

inline bool c8_stack_empty(Chip8 *self);
inline uint16_t c8_stack_peek(Chip8 *self);

/**
 * 只初始 app 碰不到的部份
 */
static Chip8 *c8_init(Chip8 *self) {
  trace("c8_new(): %p", self);
  self->pc = 0 + VM_SIZE;
  self->sp = STACK_SIZE - 1;
  self->ui = ui_new(UI_SDL, UI_WIDTH, UI_HEIGHT, 10);
  return self;
}

Chip8 *c8_new() {
  return c8_init(malloc(sizeof(Chip8)));
}

void c8_free(Chip8 *self) {
  trace("c8_free(): %p", self);
  if(self) {
    ui_free(self->ui);
    free(self);
  }
}

void c8_load(Chip8 *self, uint8_t *app, int size) {
  assert(self);
  assert(app);
  assert(size > 0 && size < USER_SIZE);
  memcpy(self->mem + APP_ENTRY, app, size);
}

static inline uint16_t c8_fetch(Chip8 *self) {
  assert(!(self->pc & 1));
  uint16_t i = self->mem[self->pc ++] << 8;
  return i | self->mem[self->pc ++];
}

static inline void c8_skip(Chip8 *self) {
  self->pc += 2;
}

static inline void c8_clear(Chip8 *self) {
}

static inline void c8_push_pc(Chip8 *self) {
  self->stack[self->sp --] = self->pc >> 8;
  self->stack[self->sp --] = self->pc & 0xff;
}

static inline void c8_pop_pc(Chip8 *self) {
  self->pc = self->stack[++ self->sp];
  self->pc |= self->stack[++ self->sp] << 8;
}

static inline void c8_jmp(Chip8 *self, uint16_t addr) {
  self->pc = addr;
}

static inline bool c8_key_pressed(Chip8 *self, int8_t key) {
  return false;
}

static inline int8_t c8_key_read(Chip8 *self) {
  return 0;
}

void c8_step(Chip8 *self) {
  assert(self);

  ui_poll_events(self->ui);

  uint16_t opcode = c8_fetch(self);
  //trace("opcode: 0x%04hx", opcode);
  switch(opcode >> 12) {
    case 0x0:
      switch(opcode & 0xfff) {
        case 0x0e0:
          trace("clear");
          c8_clear(self);
          break;
        case 0x0ee:
          trace("ret to %03hx", c8_stack_empty(self) ? 0 : c8_stack_peek(self));
          c8_pop_pc(self);
          break;
        default:
          break;
      }
      break;
    case 0x1:
      trace("jump 0x%03hx", NNN(opcode));
      c8_jmp(self, NNN(opcode));
      break;
    case 0x2:
      trace("call 0x%hx", NNN(opcode));
      c8_push_pc(self);
      c8_jmp(self, NNN(opcode));
      break;
    case 0x3:
      trace("v%hhx(%d) == %d, %s",
            VX(opcode),
            self->v[VX(opcode)],
            NNN(opcode),
            self->v[VX(opcode)] == NNN(opcode) ? "skip" : "not skip");
      if(NNN(opcode) == self->v[VX(opcode)]) {
        c8_skip(self);
      }
      break;
    case 0x4:
      trace("v%hhx(%d) != %d, %s",
            VX(opcode),
            self->v[VX(opcode)],
            NNN(opcode),
            self->v[VX(opcode)] != NNN(opcode) ? "skip" : "not skip");
      if(self->v[VX(opcode)] != NNN(opcode)) {
        c8_skip(self);
      }
      break;
    case 0x5:
      trace("v%hhx(%d) == v%hhx(%d), %s",
            VX(opcode),
            self->v[VX(opcode)],
            VY(opcode),
            self->v[VY(opcode)],
            self->v[VX(opcode)] == self->v[VY(opcode)] ? "skip" : "not skip");
      if(self->v[VX(opcode)] == self->v[VY(opcode)]) {
        c8_skip(self);
      }
      break;
    case 0x6:
      trace("v%hhx = %d", VX(opcode), (int8_t) KK(opcode));
      self->v[VX(opcode)] = KK(opcode);
      break;
    case 0x7:
      trace("v%hhx(%d) += %d", VX(opcode), (int8_t) self->v[VX(opcode)], (int8_t) KK(opcode));
      self->v[VX(opcode)] = ((int8_t) self->v[VX(opcode)]) + (int8_t) KK(opcode);
      break;
    case 0x8:
      switch(opcode & 0xf) {
        case 0x0:
          trace("v%hhx = v%hhx(%d)",
                VX(opcode),
                VY(opcode),
                (int8_t) self->v[VY(opcode)]);
          self->v[VX(opcode)] = self->v[VY(opcode)];
          break;
        case 0x1:
          trace("v%hhx(0x%02hhx) |= v%hhx(0x%02hhx) => %02hhx",
                VX(opcode),
                self->v[VX(opcode)],
                VY(opcode),
                self->v[VY(opcode)],
                self->v[VX(opcode)] | self->v[VY(opcode)]);
          self->v[VX(opcode)] |= self->v[VY(opcode)];
          break;
        case 0x2:
          trace("v%hhx(0x%02hhx) &= v%hhx(0x%02hhx) => %02hhx",
                VX(opcode),
                self->v[VX(opcode)],
                VY(opcode),
                self->v[VY(opcode)],
                self->v[VX(opcode)] & self->v[VY(opcode)]);
          self->v[VX(opcode)] &= self->v[VY(opcode)];
          break;
        case 0x3:
          trace("v%hhx(0x%02hhx) ^= v%hhx(0x%02hhx)",
                VX(opcode),
                (int8_t) self->v[VX(opcode)],
                VY(opcode),
                (int8_t) self->v[VY(opcode)]);
          self->v[VX(opcode)] ^= self->v[VY(opcode)];
          break;
        case 0x4: {
          uint16_t r = self->v[VX(opcode)] + self->v[VY(opcode)];
          trace("v%hhx(%hhu) += v%hhx(%hhu) => %hd, vf = %d",
                VX(opcode),
                self->v[VX(opcode)],
                VY(opcode),
                self->v[VY(opcode)],
                r,
                (self->v[VX(opcode)] + self->v[VY(opcode)]) > 255);
          self->v[0xf] = (r > 255);
          self->v[VX(opcode)] = r & 0xff;
          break;
        }
        case 0x5: {
          int16_t r = self->v[VX(opcode)] - self->v[VY(opcode)];
          trace("v%hhx(%d) -= v%hhx(%d) => %d, vf = %d",
                VX(opcode),
                self->v[VX(opcode)],
                VY(opcode),
                self->v[VY(opcode)],
                r,
                self->v[VX(opcode)] > self->v[VY(opcode)]);
          self->v[0xf] = self->v[VX(opcode)] > self->v[VY(opcode)];
          self->v[VX(opcode)] = r & 0xff;
          break;
        }
        case 0x6:
          trace("v%hhx(%hhx) >>= 1, vf = %d",
                VX(opcode),
                self->v[VX(opcode)],
                self->v[VX(opcode)] & 0x1);
          self->v[0xf] = self->v[VX(opcode)] & 0x1;
          self->v[VX(opcode)] >>= 1;
          break;
        case 0x7: {
          int16_t r = self->v[VY(opcode)] - self->v[VX(opcode)];
          trace("v%hhx = v%hhx(%d) - v%hhx(%d) => %d, vf = %d",
                VX(opcode),
                VY(opcode),
                self->v[VY(opcode)],
                VX(opcode),
                self->v[VX(opcode)],
                r,
                self->v[VX(opcode)] > self->v[VY(opcode)]);
          self->v[0xf] = self->v[VY(opcode)] > self->v[VX(opcode)];
          self->v[VX(opcode)] = r & 0xff;
          break;
        }
        case 0xe:
          trace("v%hhx(%hhx) <<= 1, vf = %d",
                VX(opcode),
                self->v[VX(opcode)],
                self->v[VX(opcode)] >> 7);
          self->v[0xf] = self->v[VX(opcode)] >> 7;
          self->v[VX(opcode)] <<= 1;
          break;
        default:
          assert(false);
      }
      break;
    case 0x9:
      trace("v%hhx(%d) != v%hhx(%d), %s",
            VX(opcode),
            self->v[VX(opcode)],
            VY(opcode),
            self->v[VY(opcode)],
            self->v[VX(opcode)] != self->v[VY(opcode)] ? "skip" : "not skip");
      if(self->v[VY(opcode)] != self->v[VX(opcode)]) {
        c8_skip(self);
      }
      break;
    case 0xa:
      trace("I = 0x%hx", NNN(opcode));
      self->i = NNN(opcode);
      break;
    case 0xb:
      trace("jump %hu + v0(%hhu)",
            NNN(opcode),
            self->v[0]);
      self->pc = (NNN(opcode) + self->v[0]) & 0xfff;
      break;
    case 0xc: {
      char rnd;
      getrandom(&rnd, 1, 0);
      trace("v%hhx = 0x%hhx & 0x%hhx",
            VX(opcode),
            rnd,
            KK(opcode));
      self->v[VX(opcode)] = rnd & KK(opcode);
      break;
    }
    case 0xd:
      // TODO
      break;
    case 0xe:
      switch(opcode & 0xff) {
        case 0x9e:
          if(c8_key_pressed(self, self->v[VX(opcode)])) {
            c8_skip(self);
          }
          break;
        case 0xa1:
          if(!c8_key_pressed(self, self->v[VX(opcode)])) {
            c8_skip(self);
          }
          break;
        default:
          assert(false);
      }
      break;
    case 0xf:
      switch(opcode & 0xff) {
        case 0x07:
          trace("v%hhx = dt(%hhu)", VX(opcode), self->dt);
          self->v[VX(opcode)] = self->dt;
          break;
        case 0x0a:
          self->v[VX(opcode)] = c8_key_read(self);
          break;
        case 0x15:
          trace("dt = v%hhx(%hhu)", VX(opcode), self->v[VX(opcode)]);
          self->dt = self->v[VX(opcode)];
          break;
        case 0x18:
          trace("st = v%hhx(%hhu)", VX(opcode), self->v[VX(opcode)]);
          self->st = self->v[VX(opcode)];
          break;
        case 0x1e:
          trace("I += v%hhx(%hhu)", VX(opcode), self->v[VX(opcode)]);
          self->i += (int8_t) self->v[VX(opcode)];
          break;
        case 0x29:
          // TODO
          break;
        case 0x33: {
          uint8_t l = self->v[VX(opcode)] % 10;
          uint8_t m = self->v[VX(opcode)] / 10;
          uint8_t h = m / 10;
          trace("m[%hd] = %hhd, m[%hd+1] = %hhd, m[%hd+2] = %hhd, ",
                self->i,
                h,
                self->i,
                m % 10,
                self->i,
                l);
          self->mem[self->i] = h;
          self->mem[self->i+1] = m % 10;
          self->mem[self->i+2] = l;
          break;
        }
        case 0x55:
          if(MEM_SIZE - self->i < (VX(opcode) + 1)) {
            warn("try to store %hhd registers at address %hx",
                 VX(opcode) + 1,
                 self->i);
          } else {
            memcpy(self->mem + self->i, self->v, VX(opcode));
          }
          break;
        case 0x65:
          if(MEM_SIZE - self->i < (VX(opcode) + 1)) {
            warn("try to load %hhd registers from address %hx",
                 VX(opcode) + 1,
                 self->i);
          } else {
            memcpy(self->v, self->mem + self->i, VX(opcode));
          }
          break;
        default:
          assert(false);
      }
      break;
    default:
      warn("unexpected opcode: 0x%hx", opcode);
      break;
  }
}

void c8_steps(Chip8 *self, int steps) {
  for(; steps > 0; -- steps) {
    c8_step(self);
  }
}

void c8_dump(Chip8 *self) {
  assert(self);

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

inline int16_t c8_pc(Chip8 *self) {
  assert(self);
  return self->pc;
}

inline int8_t c8_sp(Chip8 *self) {
  assert(self);
  return self->sp;
}

inline int16_t c8_i(Chip8 *self) {
  assert(self);
  return self->i;
}

inline int8_t c8_v(Chip8 *self, int v) {
  assert(self);
  assert(v < sizeof(self->v));
  return self->v[v];
}

inline int8_t c8_dt(Chip8 *self) {
  assert(self);
  return self->dt;
}

inline int8_t c8_st(Chip8 *self) {
  assert(self);
  return self->st;
}

inline uint8_t c8_mem8(Chip8 *self, int addr) {
  assert(self);
  assert(addr >= 0 && addr < MEM_SIZE);
  return *(uint8_t *)(self->mem + addr);
}

inline uint16_t c8_mem16(Chip8 *self, int addr) {
  assert(self);
  assert(addr >= 0 && addr < MEM_SIZE && (addr % 2 == 0));
  return (self->mem[addr] << 8) | self->mem[addr + 1];
}

inline bool c8_stack_empty(Chip8 *self) {
  assert(self);
  return self->sp == (STACK_SIZE - 1);
}

inline uint16_t c8_stack_peek(Chip8 *self) {
  assert(self);
  assert(!c8_stack_empty(self));
  return (self->stack[self->sp + 2] << 8) | self->stack[self->sp + 1];
}
