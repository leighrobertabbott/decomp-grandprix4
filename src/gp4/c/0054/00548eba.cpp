#include <gp4/gp4.h>

// `call 0x0052ceef ; mov [0x00831a61], eax ; ret`: counts the flagged driver bytes (eax = flag bit 7
// only, edx = both top bits) and stores eax in the (unaligned) dword 0x00831a61. Registers and flags
// are what 0x0052ceef left.
// FUNCTION: GP4 0x00548eba
void __cdecl FUN_00548eba(gp4::Regs* r) {
    gp4::call_regs(0x0052ceef, *r);
    GP4_GLOBAL(uint32_t, 0x00831a61) = r->eax;
}
GP4_IMPL(0x00548eba, FUN_00548eba, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=drv_005117d3")
