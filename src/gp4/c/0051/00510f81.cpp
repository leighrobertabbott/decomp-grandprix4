#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: runs dispatcher command 0x4e, then copies the word at 0x0082459c to 0x0066d868 and
// the dword at 0x0081d360 to 0x0066d864 (read after the command: it may change them). popal restores
// every GPR; the flags are those the dispatcher leaves (the movs do not touch them).
// FUNCTION: GP4 0x00510f81
void __cdecl FUN_00510f81(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffff0000u) | 0x4eu;
    gp4::call_regs(0x00531f9c, c);

    GP4_GLOBAL(uint16_t, 0x0066d868) = GP4_GLOBAL(uint16_t, 0x0082459c);
    GP4_GLOBAL(uint32_t, 0x0066d864) = GP4_GLOBAL(uint32_t, 0x0081d360);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00510f81, FUN_00510f81, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
