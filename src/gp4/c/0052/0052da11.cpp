#include <gp4/gp4.h>

// Runs the wanted-bitmap emptiness test 0x0052d6ed (ZF set when no bit of 0x008245a0 is set); when the bitmap
// is empty the mode byte 0x00824649 becomes 2. Registers and flags are those the test leaves.
// FUNCTION: GP4 0x0052da11
void __cdecl FUN_0052da11(gp4::Regs* r) {
    gp4::call_regs(0x0052d6ed, *r);
    if ((r->eflags & static_cast<uint32_t>(gp4::ZF)) != 0u) {
        GP4_GLOBAL(uint8_t, 0x00824649) = 2u;
    }
}
GP4_IMPL(0x0052da11, FUN_0052da11, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bitmap_0052d6ed")
