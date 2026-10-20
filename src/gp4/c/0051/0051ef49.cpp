#include <gp4/gp4.h>

// Runs dispatcher command 0x6d when the byte at 0x007a8e00 is nonzero, 0x6c otherwise
// ("cmp byte [0x7a8e00], 0; je; mov ax, 0x6d / 0x6c; call 0x00531f9c; ret"). The upper half of eax is
// untouched; eax and the flags are the dispatcher's.
// FUNCTION: GP4 0x0051ef49
void __cdecl FUN_0051ef49(gp4::Regs* r) {
    const uint32_t command = GP4_GLOBAL(uint8_t, 0x007a8e00) != 0u ? 0x6du : 0x6cu;
    r->eax = (r->eax & 0xffff0000u) | command;
    gp4::call_regs(0x00531f9c, *r);
}
GP4_IMPL(0x0051ef49, FUN_0051ef49, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
