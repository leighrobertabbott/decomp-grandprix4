#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper that inverts a 32-entry byte permutation: the table at 0x006f2d90 is first filled with
// 0xff (256 bytes), then for k = 0..31 the byte at 0x006f2d90 + [0x006f2d70 + k] is set to k. Every GPR is
// restored; the flags are those of the final `cmp ecx, 0x20` (equal).
// FUNCTION: GP4 0x0051ddaa
void __cdecl FUN_0051ddaa(gp4::Regs* r) {
    uint8_t* inverse = GP4_ARRAY(uint8_t, 0x006f2d90);
    const uint8_t* forward = GP4_ARRAY(const uint8_t, 0x006f2d70);
    for (uint32_t i = 0; i != 0x100u; ++i) {
        inverse[i] = 0xffu;
    }
    for (uint32_t k = 0; k != 0x20u; ++k) {
        inverse[forward[k]] = static_cast<uint8_t>(k);
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x20u, 0x20u));
}
GP4_IMPL(0x0051ddaa, FUN_0051ddaa, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
