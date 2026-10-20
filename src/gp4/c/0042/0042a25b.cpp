#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// `test byte ptr [0x60414c], 0xff` leaves CF = OF = AF = 0 and PF/ZF/SF of the byte. Its sign
// picks the callee, which reports its result in CF; the `stc` / `clc` tails keep that CF and the
// remaining flags of the callee. The two calls keep separate code (the second works on a copy of
// the register file) so the compiler cannot fold them into one call with a computed address.
// (esi is a car record of the 22-record array at 0x66fef0.)
// FUNCTION: GP4 0x0042a25b
void __cdecl FUN_0042a25b(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint8_t, 0x0060414c);
    const uint32_t flags = parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
    if ((value & 0x80u) != 0u) {
        gp4::call_regs(0x0042ba87, *r);
    } else {
        gp4::Regs callee = *r;
        gp4::call_regs(0x0042bae4, callee);
        *r = callee;
    }
}
GP4_IMPL(0x0042a25b, FUN_0042a25b, "regs(esi:rec[0x66fef0,0x41c,0..21], ebx:u32) -> (ebx, cf, pf, af, zf, sf, of); globals=layout=state_00403800+cars_0066fef0")
