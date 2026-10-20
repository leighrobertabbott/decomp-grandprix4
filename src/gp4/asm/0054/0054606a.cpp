#include <gp4/gp4.h>
#include <gp4/flags.h>

// Advances the cursor in esi and decrements the dword counter at 0x00831564, then calls 0x0054633d
// (a register- and flag-preserving routine). The flags are those of `sub dword [0x831564], 1`.
// FUNCTION: GP4 0x0054606a
void __cdecl FUN_0054606a(gp4::Regs* r) {
    r->esi = r->esi + 1u;
    const uint32_t old = GP4_GLOBAL(uint32_t, 0x00831564);
    GP4_GLOBAL(uint32_t, 0x00831564) = old - 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(old, 1u));
    gp4::call_regs(0x0054633d, *r);
}
GP4_IMPL(0x0054606a, FUN_0054606a, "regs(esi:u32) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
