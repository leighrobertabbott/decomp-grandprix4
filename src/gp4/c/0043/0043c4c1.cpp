#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043c4c1
void __cdecl FUN_0043c4c1(gp4::Regs* r) {
    // Rounded division of the dword at 0x0066d814 by 60000 (one minute in ms):
    // (value + 30000) / 60000 with edx:eax = 0:sum, so the dividend is unsigned.
    // ebp and edx are saved and restored. idiv leaves the flags of "xor edx, edx": ZF|PF.
    const uint32_t sum = 0xea60u / 2u + GP4_GLOBAL(uint32_t, 0x0066d814);
    r->eax = sum / 0xea60u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043c4c1, FUN_0043c4c1, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
