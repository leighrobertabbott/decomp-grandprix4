#include <gp4/gp4.h>
#include <gp4/flags.h>

// esi += 2 on both paths of `cmp byte [0x86857c], 2; je`. The cmp flags are overwritten by the
// add, so the flags are those of `add esi, 2` (the mode byte is a dead read, specs/shape_exempt.json).
// FUNCTION: GP4 0x005571d5
void __cdecl FUN_005571d5(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    r->esi = esi + 2u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 2u));
}
GP4_IMPL(0x005571d5, FUN_005571d5, "regs(esi:u32) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
