#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the first dword of each of the eight 0x1c-byte slots 0x0081cd91..0x0081ce6d, then stores the value of
// [0x007a9478] into the dword 0x0081ce71 and 0xffffffff into 0x0081ce75. eax is saved and restored; the flags are
// those of the final `cmp eax, 0x81ce71` (equal).
// FUNCTION: GP4 0x00528114
void __cdecl FUN_00528114(gp4::Regs* r) {
    uint32_t slot = 0x0081cd91u;
    do {
        GP4_GLOBAL(uint32_t, slot) = 0u;
        slot += 0x1cu;
    } while (slot != 0x0081ce71u);

    GP4_GLOBAL(uint32_t, 0x0081ce71) = GP4_GLOBAL(uint32_t, 0x007a9478);
    GP4_GLOBAL(uint32_t, 0x0081ce75) = 0xffffffffu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(slot, 0x0081ce71u));
}
GP4_IMPL(0x00528114, FUN_00528114, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
