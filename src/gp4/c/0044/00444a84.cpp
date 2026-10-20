#include <gp4/gp4.h>

// Squares, in place and modulo 2^32, the first three dwords of each of the 23 four-dword entries (stride 0x10) of
// the static inertia/mass table at 0x0062a424; the fourth dword (the mass) is left alone. pushal / popal restore
// every register; the flags are those of the final `cmp ecx, 0x17` (equal: ZF | PF).
// FUNCTION: GP4 0x00444a84
void __cdecl FUN_00444a84(gp4::Regs* r) {
    uint32_t* entry = GP4_ARRAY(uint32_t, 0x0062a424);
    for (uint32_t i = 0; i != 0x17u; ++i) {
        entry[0] = entry[0] * entry[0];
        entry[1] = entry[1] * entry[1];
        entry[2] = entry[2] * entry[2];
        entry += 4;
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00444a84, FUN_00444a84, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
