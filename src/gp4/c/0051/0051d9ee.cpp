#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the mode byte 0x007aafdb is 1, stores the high-resolution timer fraction of 0x0054e6af in
// the dword 0x006f2c00 (eax keeps that value). The flags are those of `cmp byte [0x7aafdb], 1`, or
// those 0x0054e6af leaves when it ran.
// FUNCTION: GP4 0x0051d9ee
void __cdecl FUN_0051d9ee(gp4::Regs* r) {
    uint32_t flags = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007aafdb), 1u);
    if (GP4_GLOBAL(uint8_t, 0x007aafdb) == 1u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        gp4::call_regs(0x0054e6af, c);
        GP4_GLOBAL(uint32_t, 0x006f2c00) = c.eax;
        r->eax = c.eax;
        flags = c.eflags;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051d9ee, FUN_0051d9ee, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
