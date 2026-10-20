// gp4/flags.h - arithmetic-flag results of single x86 instructions, for regs-convention
// reconstructions that must return the flags the original leaves (the oracle compares
// CF PF AF ZF SF OF bit-exactly).
//
//   uint32_t f = gp4::flags::sub32(a, b);          // flags of `cmp a, b` / `sub a, b`
//   r->eflags  = gp4::flags::merge(r->eflags, f);  // keep the non-arithmetic bits
//
// Logic instructions (and/or/xor/test) clear CF and OF; AF is architecturally undefined
// there and is modelled as 0, matching the pinned Unicorn oracle. inc/dec keep CF.
// Verified against the host CPU by tests/native/test_flags.cpp (defined flags).
#pragma once
#include <stdint.h>

namespace gp4 {
namespace flags {

enum : uint32_t { CF = 1u << 0, PF = 1u << 2, AF = 1u << 4, ZF = 1u << 6, SF = 1u << 7,
                  OF = 1u << 11, ARITH = 0x8d5u };

inline uint32_t parity(uint32_t v) {
    v &= 0xffu;
    v ^= v >> 4;
    v ^= v >> 2;
    v ^= v >> 1;
    return (v & 1u) ? 0u : PF;
}

// ZF/SF/PF of a result of `bits` width (8, 16 or 32).
inline uint32_t szp(uint32_t r, int bits) {
    const uint32_t mask = bits == 32 ? 0xffffffffu : ((1u << bits) - 1u);
    r &= mask;
    return (r == 0u ? ZF : 0u) | (((r >> (bits - 1)) & 1u) ? SF : 0u) | parity(r);
}

inline uint32_t sub(uint32_t a, uint32_t b, int bits) {
    const uint32_t mask = bits == 32 ? 0xffffffffu : ((1u << bits) - 1u);
    a &= mask;
    b &= mask;
    const uint32_t r = (a - b) & mask;
    const uint32_t sign = 1u << (bits - 1);
    return (a < b ? CF : 0u) | szp(r, bits) | (((a ^ b ^ r) & 0x10u) ? AF : 0u)
         | ((((a ^ b) & (a ^ r)) & sign) ? OF : 0u);
}

inline uint32_t add(uint32_t a, uint32_t b, int bits) {
    const uint32_t mask = bits == 32 ? 0xffffffffu : ((1u << bits) - 1u);
    a &= mask;
    b &= mask;
    const uint32_t r = (a + b) & mask;
    const uint32_t sign = 1u << (bits - 1);
    return (r < a ? CF : 0u) | szp(r, bits) | (((a ^ b ^ r) & 0x10u) ? AF : 0u)
         | (((~(a ^ b) & (a ^ r)) & sign) ? OF : 0u);
}

inline uint32_t sub32(uint32_t a, uint32_t b) { return sub(a, b, 32); }   // cmp/sub
inline uint32_t sub16(uint32_t a, uint32_t b) { return sub(a, b, 16); }
inline uint32_t sub8(uint32_t a, uint32_t b) { return sub(a, b, 8); }
inline uint32_t add32(uint32_t a, uint32_t b) { return add(a, b, 32); }
inline uint32_t add16(uint32_t a, uint32_t b) { return add(a, b, 16); }
inline uint32_t add8(uint32_t a, uint32_t b) { return add(a, b, 8); }
inline uint32_t neg32(uint32_t a) { return sub(0u, a, 32); }              // CF = (a != 0)
inline uint32_t neg16(uint32_t a) { return sub(0u, a, 16); }
// and/or/xor/test result r: CF = OF = AF = 0.
inline uint32_t logic32(uint32_t r) { return szp(r, 32); }
inline uint32_t logic16(uint32_t r) { return szp(r, 16); }
inline uint32_t logic8(uint32_t r) { return szp(r, 8); }
// inc/dec keep the incoming CF.
inline uint32_t inc32(uint32_t a, uint32_t in_flags) { return (add(a, 1u, 32) & ~CF) | (in_flags & CF); }
inline uint32_t dec32(uint32_t a, uint32_t in_flags) { return (sub(a, 1u, 32) & ~CF) | (in_flags & CF); }
inline uint32_t inc16(uint32_t a, uint32_t in_flags) { return (add(a, 1u, 16) & ~CF) | (in_flags & CF); }
inline uint32_t dec16(uint32_t a, uint32_t in_flags) { return (sub(a, 1u, 16) & ~CF) | (in_flags & CF); }

// Replace the six arithmetic flags of `eflags` with `arith`.
inline uint32_t merge(uint32_t eflags, uint32_t arith) { return (eflags & ~ARITH) | (arith & ARITH); }

}  // namespace flags
}  // namespace gp4
