#include <gp4/gp4.h>
#include <gp4/flags.h>

// Skips a dword in the cursor esi (`add esi, 4; ret`); the flags are those of the addition.
// FUNCTION: GP4 0x005582f7
void __cdecl FUN_005582f7(gp4::Regs* r) {
    const uint32_t before = r->esi;
    r->esi = before + 4u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 4u));
}
GP4_IMPL(0x005582f7, FUN_005582f7, "regs(esi:u32) -> (esi, cf, pf, af, zf, sf, of)")
