#include <gp4/gp4.h>

// Runs the bit-set synchroniser 0x00510e2d for bit 0x1b (byte 3, mask 8 of the bitmaps 0x008245a0 /
// 0x008910d5), then `jne`: ZF set (state bit newly set, cl = 0) takes `stc`, otherwise `clc`.
// ecx is saved and restored; the other flags are those 0x00510e2d leaves, CF is replaced.
// FUNCTION: GP4 0x0051f1c1
void __cdecl FUN_0051f1c1(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.ecx = 0x1bu;
    gp4::call_regs(0x00510e2d, c);
    const uint32_t carry = (c.eflags & gp4::ZF) != 0u ? 1u : 0u;
    r->eflags = (c.eflags & ~1u) | carry;
}
GP4_IMPL(0x0051f1c1, FUN_0051f1c1, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bits_00510e2d")
