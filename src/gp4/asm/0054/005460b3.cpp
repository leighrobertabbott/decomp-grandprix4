#include <gp4/gp4.h>
#include <gp4/flags.h>

// `inc esi`, then 0x005461b9 (draws the string at esi with the pair at 0x831560/64, through 0x508400 or
// 0x54657d depending on the byte at 0x7ab53c), then 0x00546154 (skips the cursor over bytes >= 0x14).
// Registers and flags are whatever the second call leaves.
// FUNCTION: GP4 0x005460b3
void __cdecl FUN_005460b3(gp4::Regs* r) {
    const uint32_t before = r->esi;
    r->esi = before + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(before, r->eflags));
    gp4::call_regs(0x005461b9, *r);
    gp4::call_regs(0x00546154, *r);
}
GP4_IMPL(0x005460b3, FUN_005460b3, "regs(esi:ptr[0x200]:bytes) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
