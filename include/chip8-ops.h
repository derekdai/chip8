#include <stdint.h>

#ifndef __CHIP8_OPS_
#define __CHIP8_OPS_

// SYS addr
#define OP_0nnn(nnn) (nnn) >> 8, (nnn) & 0xff 
#define OP_NOP OP_0nnn(0)

// CLS
#define OP_00E0 0x0, 0xe0

// RET
// return from subroutine
#define OP_00EE 0x0, 0xee

// JP addr
// jump to nnn
#define OP_1nnn(nnn) 0x10 | ((nnn) >> 8), (nnn) & 0xff

// CALL addr
// push current pc then call to nnn
#define OP_2nnn(nnn) 0x20 | ((nnn) >> 8), (nnn) & 0xff

// SE Vx, byte
// Vx == kk, skip next instruction
#define OP_3xkk(x, kk) 0x30 | ((x) & 0xf), (kk) & 0xff

// SNE Vx, kk
// Vx != kk, skip next instruction
#define OP_4xkk(x, kk) 0x40 | ((x) & 0xf), (kk) & 0xff

// SE Vx, Vy
// Vx == Vy, skip next instruction
#define OP_5xy0(x, y) 0x50 | ((x) & 0xf), ((y) & 0xf) << 4

// LD Vx, byte
// Vx = kk
#define OP_6xkk(x, kk) 0x60 | ((x) & 0xf), (kk) & 0xff

// ADD Vx, kk
// Vx += kk
#define OP_7xkk(x, kk) 0x70 | ((x) & 0xf), (kk) & 0xff

// LD Vx, Vy
// Vx = Vy
#define OP_8xy0(x, y) 0x80 | ((x) & 0xf), (((y) & 0xf) << 4)

// OR Vx, Vy
// Vx |= Vy
#define OP_8xy1(x, y) 0x80 | ((x) & 0xf), (((y) & 0xf) << 4) | 0x1

// AND Vx, Vy
// Vx &= Vy
#define OP_8xy2(x, y) 0x80 | ((x) & 0xf), (((y) & 0xf) << 4) | 0x2

// XOR Vx, Vy
// Vx ^= Vy
#define OP_8xy3(x, y) 0x80 | ((x) & 0xf), ((y) << 4) | 0x3

// ADD Vx, Vy
// Vx += Vy, VF = carry
#define OP_8xy4(x, y) 0x80 | ((x) & 0xf), ((y) << 4) | 0x4

// SUB Vx, Vy
// Vx -= Vy, VF = not borrow
#define OP_8xy5(x, y) 0x80 | ((x) & 0xf), ((y) << 4) | 0x5

// SHR Vx {, Vy}
// Vx >>= 1, VF = least-significant bit
#define OP_8xy6(x) 0x80 | ((x) & 0xf), 0x6

// SUBN Vx, Vy
// Vx = Vy - Vx, VF = not borrow
#define OP_8xy7(x, y) 0x80 | ((x) & 0xf), ((y) << 4) | 0x7

// SHL Vx {, Vy}
// Vx <<= 1, VF = most-significant bit
#define OP_8xye(x) 0x80 | ((x) & 0xf), 0xe

// SNE Vx, Vy
// Vx != Vy, skip next instruction
#define OP_9xy0(x, y) 0x90 | ((x) & 0xf), ((y) & 0xf) << 4

// LD I, addr
// I = nnn
#define OP_annn(nnn) 0xa0 | ((nnn) >> 8), (nnn) & 0xff

// JP V0, addr
// jump to v0 + nnn
#define OP_bnnn(nnn) 0xb0 | ((nnn) >> 8), (nnn) & 0xff

// RND Vx, byte
// Vx = random byte & kk
#define OP_cxkk(x, kk) 0xc0 | ((x) & 0xf), (kk) & 0xff

// LD Vx, DT
// Vx = DT
#define OP_fx07(x) 0xf0 | ((x) & 0xf), 0x7

// LD DT, Vx
// DT = Vx
#define OP_fx15(x) 0xf0 | ((x) & 0xf), 0x15

// LD ST, Vx
// ST = Vx
#define OP_fx18(x) 0xf0 | ((x) & 0xf), 0x18

// ADD I, Vx
// I += Vx
#define OP_fx1e(x) 0xf0 | ((x) & 0xf), 0x1e

// LD B, Vx
// mem[I] = Vx / 100, mem[I+1] = (Vx / 10) % 10, mem[I+2] = Vx % 10
#define OP_fx1e(x) 0xf0 | ((x) & 0xf), 0x1e

// LD B, Vx
// mem[I] = Vx / 100, mem[I+1] = (Vx / 10) % 10, mem[I+2] = Vx % 10
#define OP_fx1e(x) 0xf0 | ((x) & 0xf), 0x1e

#endif /* __CHIP8_OPS_ */
