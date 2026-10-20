#include <gp4/gp4.h>
#include <gp4/flags.h>

// esi += 6 on both paths of `cmp byte [0x86857c], 2; je`. The cmp flags are overwritten by the
// add, so the flags are those of `add esi, 6` (the mode byte is a dead read, specs/shape_exempt.json).
// FUNCTION: GP4 0x0055878f
void __cdecl FUN_0055878f(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    r->esi = esi + 6u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 6u));
}
GP4_IMPL(0x0055878f, FUN_0055878f, "regs(esi:u32) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
