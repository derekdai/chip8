#include <assert.h>
#include "logging.h"
#include "chip8.h"
#include "chip8-ops.h"

int main() {
  {
    AutoChip8 *vm = c8_new();
    assert(c8_pc(vm) == APP_ENTRY);
    c8_load(vm, (uint8_t[]){OP_NOP}, 2);
    c8_step(vm);
    assert(c8_pc(vm) == (APP_ENTRY + sizeof(int16_t)));
  }

  {
    AutoChip8 *vm = c8_new();
    c8_load(vm, (uint8_t[]){OP_1nnn(0x234)}, 2);
    assert(c8_pc(vm) == APP_ENTRY);
    c8_step(vm);
    assert(c8_pc(vm) == 0x234);
  }

  {
    AutoChip8 *vm = c8_new();
    c8_load(vm, (uint8_t[]){OP_2nnn(0x202), OP_00EE}, 4);
    assert(c8_pc(vm) == APP_ENTRY);
    assert(c8_stack_empty(vm));
    c8_step(vm);
    assert(c8_pc(vm) == 0x202);
    assert(!c8_stack_empty(vm));
    assert(c8_stack_peek(vm) == 0x202);
    c8_step(vm);
    assert(c8_pc(vm) == 0x202);
    assert(c8_stack_empty(vm));
  }

  {
    AutoChip8 *vm = c8_new();
    c8_load(vm, (uint8_t[]){OP_8xy3(0, 0)}, 2);
    c8_step(vm);
    assert(c8_v(vm, 0) == 0);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_8xy3(0, 0),
      OP_3xkk(0, 0)
    };
    c8_load(vm, ops, sizeof(ops));
    c8_step(vm);
    c8_step(vm);
    assert(c8_pc(vm) == (0x200 + 2 + 2 + 2));
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_8xy3(0, 0),
      OP_3xkk(0, 1)
    };
    c8_load(vm, ops, sizeof(ops));
    c8_step(vm);
    c8_step(vm);
    assert(c8_pc(vm) == (0x200 + 2 + 2));
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_8xy3(0, 0),
      OP_4xkk(0, 1)
    };
    c8_load(vm, ops, sizeof(ops));
    c8_step(vm);
    c8_step(vm);
    assert(c8_pc(vm) == (0x200 + 2 + 2 + 2));
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_8xy3(0, 0),
      OP_4xkk(0, 0)
    };
    c8_load(vm, ops, sizeof(ops));
    c8_step(vm);
    c8_step(vm);
    assert(c8_pc(vm) == (0x200 + 2 + 2));
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(0, 123),
      OP_6xkk(1, 111),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_step(vm);
    c8_step(vm);
    assert(c8_v(vm, 0) == 123);
    assert(c8_v(vm, 1) == 111);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_8xy3(2, 2),
      OP_7xkk(2, 1),
      OP_7xkk(2, -1),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_step(vm);
    assert(c8_v(vm, 2) == 0);
    c8_step(vm);
    assert(c8_v(vm, 2) == 1);
    c8_step(vm);
    assert(c8_v(vm, 2) == 0);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(4, 0xf0),
      OP_6xkk(5, 0x0f),
      OP_8xy1(4, 5),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_step(vm);
    assert(c8_v(vm, 4) == (int8_t) 0xf0);
    c8_step(vm);
    assert(c8_v(vm, 5) == 0xf);
    c8_step(vm);
    assert(c8_v(vm, 4) == (int8_t) 0xff);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(4, 0xff),
      OP_6xkk(5, 0x0f),
      OP_8xy2(4, 5),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 3);
    assert(c8_v(vm, 4) == 0xf);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(4, 0xff),
      OP_6xkk(5, 0xff),
      OP_8xy4(4, 5),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 3);
    assert(c8_flag(vm) == 1);
    assert(c8_v(vm, 4) == (int8_t) ((0xff + 0xff) & 0xff));
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(4, 0xfe),
      OP_6xkk(5, 0x1),
      OP_8xy4(4, 5),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 3);
    assert(c8_flag(vm) == 0);
    assert(c8_v(vm, 4) == (int8_t) 0xff);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(4, 123),
      OP_6xkk(5, 23),
      OP_8xy5(4, 5),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 3);
    assert(c8_flag(vm) == 1);
    assert(c8_v(vm, 4) == 100);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(4, 23),
      OP_6xkk(5, 123),
      OP_8xy5(4, 5),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 3);
    assert(c8_flag(vm) == 0);
    assert(c8_v(vm, 4) == -100);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(4, 23),
      OP_6xkk(5, 123),
      OP_8xy5(4, 5),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 3);
    assert(c8_flag(vm) == 0);
    assert(c8_v(vm, 4) == -100);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(6, 2),
      OP_8xy6(6),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 2);
    assert(c8_flag(vm) == 0);
    assert(c8_v(vm, 6) == 1);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(6, 3),
      OP_8xy6(6),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 2);
    assert(c8_flag(vm) == 1);
    assert(c8_v(vm, 6) == 1);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(8, 23),
      OP_6xkk(9, 123),
      OP_8xy7(8, 9),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 3);
    assert(c8_flag(vm) == 1);
    assert(c8_v(vm, 8) == 100);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(8, 123),
      OP_6xkk(9, 23),
      OP_8xy7(8, 9),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 3);
    assert(c8_flag(vm) == 0);
    assert(c8_v(vm, 8) == -100);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(6, 0x80),
      OP_8xye(6),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 2);
    assert(c8_flag(vm) == 1);
    assert(c8_v(vm, 6) == 0);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(6, 0x40),
      OP_8xye(6),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 2);
    assert(c8_flag(vm) == 0);
    assert(c8_v(vm, 6) == (int8_t) 0x80);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(0, 0x40),
      OP_6xkk(1, 0x40),
      OP_9xy0(0, 1),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 3);
    assert(c8_pc(vm) == APP_ENTRY + 2 * 3);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(0, 0x40),
      OP_6xkk(1, 0x41),
      OP_9xy0(0, 1),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 3);
    assert(c8_pc(vm) == APP_ENTRY + 2 * 3 + 2);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(0, 0),
      OP_bnnn(0x202),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 2);
    assert(c8_pc(vm) == 0x202);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(0, 0x10),
      OP_bnnn(0x200),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 2);
    assert(c8_pc(vm) == 0x210);
  }

  {
    AutoChip8 *vm = c8_new();
    uint8_t ops[] = {
      OP_6xkk(0, 0),
      OP_cxkk(0, 0xff),
      OP_6xkk(1, 0),
      OP_cxkk(1, 0xff),
      OP_6xkk(2, 0),
      OP_cxkk(2, 0xff),
      OP_cxkk(3, 0),
    };
    c8_load(vm, ops, sizeof(ops));
    c8_steps(vm, 7);
    assert(c8_v(vm, 0) != (c8_v(vm, 1) ^ c8_v(vm, 2)));
    assert(c8_v(vm, 3) == 0);
  }
}
