#include <gp4/gp4.h>
#include <gp4/flags.h>

// esi += 0x18 on both paths of `cmp byte [0x86857c], 1; je`. The cmp flags are overwritten by the
// add, so the flags are those of `add esi, 0x18`.
static __declspec(noinline) uint32_t ModeMatches(uint8_t mode) {
    return mode == 1u ? 1u : 0u;
}

// FUNCTION: GP4 0x00555e93
void __cdecl FUN_00555e93(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    uint32_t flags = gp4::flags::add32(esi, 0x18u);
    if (ModeMatches(GP4_GLOBAL(uint8_t, 0x0086857c))) {
        r->esi = esi + 0x18u;
    } else {
        r->esi = esi + 0x18u;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00555e93, FUN_00555e93, "regs(esi:u32) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
