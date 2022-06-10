#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include "config.h"
#include "logging.h"
#include "chip8.h"
#include "ui.h"

#ifdef ENABLE_DTRACE
#include "chip8-sdt.h"
#else
#define CHIP8_EXEC_BEGIN()
#define CHIP8_EXEC_END()
#define CHIP8_ILLEGAL_OPCODE(op)
#endif

#define VX(op) ((uint8_t)((op) >> 8) & 0xf)
#define VY(op) ((uint8_t)((op) >> 4) & 0xf)
#define NNN(op) ((uint16_t)(op) & 0xfff)
#define N(op) ((uint8_t)(op) & 0xf)
#define KK(op) ((uint8_t)(op) & 0xff)

struct _Chip8 {
  Ui *ui;
  bool dirty;

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
  self->sp = STACK_SIZE;
  self->ui = ui_new(UI_SDL, UI_WIDTH, UI_HEIGHT, 16);
  self->dirty = false;
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

static inline OpCode c8_fetch(Chip8 *self) {
  OpCode op;
  assert(!(self->pc & 1));
  op = self->mem[self->pc ++] << 8;
  return op | self->mem[self->pc ++];
}

static inline void c8_skip(Chip8 *self) {
  self->pc += 2;
}

static inline void c8_fb_clear(Chip8 *self) {
  memset(self->fb, 0, FRAMEBUFFER_SIZE);
  self->dirty = true;
}

static const char *to_bin(uint8_t v, char *buf) {
  int i;
  for(i = 0; i < 8; ++ i) {
    buf[i] = v & (1 << (7 - i)) ? '#' : ' ';
  }
  buf[8] = '\0';
  return buf;
}

static void c8_fb_draw(Chip8 *self, int x, int y, int n) {
  int i;
  int bitshift = x & 0x7;
  for(i = 0; i < n; ++ i) {
    int dist = (y + i) * UI_WIDTH + x;
    uint8_t *fb = self->fb + (dist >> 3);
    uint8_t v = self->mem[self->i + i];
    dump("x=%3u, y=%3u, dist=%4d, shift=%d, v=%s, r=%s%s",
          x,
          y + i,
          dist,
          bitshift,
          to_bin(v, (char[9]){}),
          to_bin(v >> bitshift, (char[9]){}),
          to_bin(v << (8 - bitshift), (char[9]){}));
    *fb ^= v >> bitshift;
    if(bitshift) {
      *(fb + 1) ^= v << (8 - bitshift);
    }
  }

  //int r, c;
  //for(r = 0; r < 32; r ++) {
  //  for(c = 0; c < 8; c ++) {
  //    printf("%s", to_bin(self->fb[r * (UI_WIDTH >> 3) + c], (char[9]){}));
  //  }
  //  printf("\n");
  //}

  self->dirty = true;
}

static inline void c8_push_pc(Chip8 *self) {
  self->stack[-- self->sp] = self->pc >> 8;
  self->stack[-- self->sp] = self->pc & 0xff;
}

static inline void c8_pop_pc(Chip8 *self) {
  self->pc = self->stack[self->sp ++];
  self->pc |= self->stack[self->sp ++] << 8;
}

static inline void c8_jmp(Chip8 *self, uint16_t addr) {
  self->pc = addr;
}

static inline bool c8_key_pressed(Chip8 *self, int8_t key) {
  return ui_key_pressed(self->ui, key & 0xf);
}

static inline int8_t c8_key_wait(Chip8 *self) {
  return 0;
}

void c8_step(Chip8 *self) {
  OpCode opcode;

  assert(self);

  CHIP8_EXEC_BEGIN();

  ui_poll_events(self->ui);

  opcode = c8_fetch(self);
  trace("opcode: 0x%04hx", opcode);
  switch(opcode >> 12) {
    case 0x0:
      switch(opcode & 0xfff) {
        case 0x0e0:
          trace("clear");
          c8_fb_clear(self);
          break;
        case 0x0ee:
          trace("ret to %03hx", c8_stack_empty(self) ? 0 : c8_stack_peek(self));
          c8_pop_pc(self);
          break;
        default:
          CHIP8_ILLEGAL_OPCODE(opcode);
          return;
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
            KK(opcode),
            self->v[VX(opcode)] == KK(opcode) ? "skip" : "not skip");
      if(KK(opcode) == self->v[VX(opcode)]) {
        c8_skip(self);
      }
      break;
    case 0x4:
      trace("v%hhx(%d) != %d, %s",
            VX(opcode),
            self->v[VX(opcode)],
            KK(opcode),
            self->v[VX(opcode)] != KK(opcode) ? "skip" : "not skip");
      if(self->v[VX(opcode)] != KK(opcode)) {
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
          CHIP8_ILLEGAL_OPCODE(opcode);
          return;
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
    case 0xd: {
      trace("DRW v%hhx(%hhu), v%hhx(%hhu) <= I(%hu), %hhu",
            VX(opcode),
            self->v[VX(opcode)],
            VY(opcode),
            self->v[VY(opcode)],
            self->i,
            N(opcode));
      c8_fb_draw(self, self->v[VX(opcode)], self->v[VY(opcode)], N(opcode));
      break;
    }
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
          CHIP8_ILLEGAL_OPCODE(opcode);
          return;
      }
      break;
    case 0xf:
      switch(opcode & 0xff) {
        case 0x07:
          trace("v%hhx = dt(%hhu)", VX(opcode), self->dt);
          self->v[VX(opcode)] = self->dt;
          break;
        case 0x0a:
          self->v[VX(opcode)] = c8_key_wait(self);
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
            trace("store v0-v%uux on I(0x%hx)",
                  VX(opcode) + 1,
                  self->i);
            memcpy(self->mem + self->i, self->v, VX(opcode) + 1);
            self->i += VX(opcode) + 1;
          }
          break;
        case 0x65:
          if(MEM_SIZE - self->i < (VX(opcode) + 1)) {
            warn("try to load %hhd registers from address %hx",
                 VX(opcode) + 1,
                 self->i);
          } else {
            trace("load v0-v%uux on I(0x%hx)",
                  VX(opcode) + 1,
                  self->i);
            memcpy(self->v, self->mem + self->i, VX(opcode) + 1);
            self->i += VX(opcode) + 1;
          }
          break;
        default:
          CHIP8_ILLEGAL_OPCODE(opcode);
          return;
      }
      break;
    default:
      warn("unexpected opcode: 0x%hx", opcode);
      break;
  }

  if(self->dirty) {
    ui_flush(self->ui, self->fb);
    self->dirty = false;
  }

  CHIP8_EXEC_END();
}

void c8_steps(Chip8 *self, int steps) {
  for(; steps > 0; -- steps) {
    c8_step(self);
  }
}

void c8_dump(Chip8 *self) {
  assert(self);

  info("\n{ pc: %04d, sp: %04d, i: %04d, dt: %03d, st: %03d, \n" \
       "  v0: %03d, v1: %03d, v2: %03d, v3: %03d, \n" \
       "  v4: %03d, v5: %03d, v6: %03d, v7: %03d, \n" \
       "  v8: %03d, v9: %03d, va: %03d, vb: %03d, \n" \
       "  vc: %03d, vd: %03d, ve: %03d, vf: %03d }",
       self->pc, self->sp, self->i, self->dt, self->st,
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

inline int8_t c8_v(Chip8 *self, uint8_t v) {
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
  return self->sp == STACK_SIZE;
}

inline uint16_t c8_stack_peek(Chip8 *self) {
  assert(self);
  assert(!c8_stack_empty(self));
  return (self->stack[self->sp + 1] << 8) | self->stack[self->sp];
}
