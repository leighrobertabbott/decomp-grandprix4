#include <gp4/gp4.h>
#include <gp4/flags.h>

// Advances the cursor in esi by one (`inc esi; ret`). inc keeps the incoming CF and rewrites the other
// arithmetic flags from the incremented value.
// FUNCTION: GP4 0x00546078
void __cdecl FUN_00546078(gp4::Regs* r) {
    const uint32_t before = r->esi;
    r->esi = before + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(before, r->eflags));
}
GP4_IMPL(0x00546078, FUN_00546078, "regs(esi:u32, cf:u32[0..1]) -> (esi, cf, pf, af, zf, sf, of)")
