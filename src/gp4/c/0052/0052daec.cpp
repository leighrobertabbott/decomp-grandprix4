#include <gp4/gp4.h>

// Runs the wanted-bitmap emptiness test 0x0052d6ed (ZF set when no bit of 0x008245a0 is set); when the bitmap
// is empty the mode byte 0x00824649 becomes 10 and the linked-list flag setter 0x0053e612 runs with
// esi = 0x0084bc2a (the list head) and eax = 4. That routine restores every register, so eax = 4 and
// esi = 0x0084bc2a remain; the flags are whatever it leaves.
// FUNCTION: GP4 0x0052daec
void __cdecl FUN_0052daec(gp4::Regs* r) {
    gp4::call_regs(0x0052d6ed, *r);
    if ((r->eflags & static_cast<uint32_t>(gp4::ZF)) != 0u) {
        GP4_GLOBAL(uint8_t, 0x00824649) = 10u;
        r->esi = 0x0084bc2au;
        r->eax = 4u;
        gp4::call_regs(0x0053e612, *r);
    }
}
GP4_IMPL(0x0052daec, FUN_0052daec, "regs() -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_0052dba4+bitmap_0052d6ed")
