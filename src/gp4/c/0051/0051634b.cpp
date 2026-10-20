#include <gp4/gp4.h>
#include <gp4/flags.h>

// Latches the "qualifying record" test of 0x40e802 for the selected record
// [0x679d30] when the mode byte 0x7c2ea9 equals 0x80: the returned value is
// stored at 0xa6987c and 0xa69878 becomes 1 when the routine reports CF = 1
// (it is cleared first otherwise). pushal/popal keep every register; popal does
// not restore flags, so the flags are those of the last flag writer: the byte
// compare, or the flags 0x40e802 returned.

// FUNCTION: GP4 0x0051634b
void __cdecl FUN_0051634b(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00a69878) = 0;
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags = gp4::flags::sub8(mode, 0x80u);
    if (mode == 0x80) {
        gp4::Regs c = {};
        c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
        c.eflags = r->eflags;
        gp4::call_regs(0x0040e802, c);
        GP4_GLOBAL(uint32_t, 0x00a6987c) = c.eax;
        flags = c.eflags;
        if (c.eflags & gp4::CF) {
            GP4_GLOBAL(uint32_t, 0x00a69878) = 1;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051634b, FUN_0051634b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0044334d+clock_0066d814+sel_0051634b")
