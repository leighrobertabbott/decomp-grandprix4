#include <gp4/gp4.h>

// Returns the short constant 0x7fff in ax and leaves the upper half of eax untouched:
// `mov ax, 0x7fff; ret`. The callers read all of eax, so the whole register is declared.
// FUNCTION: GP4 0x005a7a70
void __cdecl FUN_005a7a70(gp4::Regs* r) {
    const uint32_t eaxIn = r->eax;
    r->eax = (eaxIn & 0xffff0000u) | 0x7fffu;
}
GP4_IMPL(0x005a7a70, FUN_005a7a70, "regs(eax:u32) -> (eax)")
