#include <gp4/gp4.h>

// Clears the 0x16c0 bytes at 0x00637f60..0x00639620 and then stores 0.0 to the five qwords at
// 0x00638350..0x00638378 (inside the cleared range). ebx is saved and restored. The scan ends on
// `cmp ebx, 0x638378` with ebx equal, so the flags are ZF = PF = 1 and the rest clear.
// FUNCTION: GP4 0x00411f73
void __cdecl FUN_00411f73(gp4::Regs* r) {
    uint8_t* bytes = GP4_ARRAY(uint8_t, 0x00637f60);
    do {
        *bytes = 0;
        ++bytes;
    } while (bytes != GP4_ARRAY(uint8_t, 0x00639620));

    for (double* slot = GP4_ARRAY(double, 0x00638350); slot < GP4_ARRAY(double, 0x00638378); ++slot) {
        *slot = 0.0;
    }

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00411f73, FUN_00411f73, "regs() -> (cf, pf, af, zf, sf, of)")
