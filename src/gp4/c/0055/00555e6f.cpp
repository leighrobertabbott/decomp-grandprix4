#include <gp4/gp4.h>
#include <gp4/flags.h>

// Advances esi by 4 past a two-word entry {a, b} (flags of that add) in every mode. When the mode
// byte 0x0086857c is 1 it also stores a (zero-extended) in [0x00874df4] and b (sign-extended) in
// [0x00874dfc]; the body is a pushal/popal pair, so eax and the other registers are unchanged.
// FUNCTION: GP4 0x00555e6f
void __cdecl FUN_00555e6f(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        GP4_GLOBAL(uint32_t, 0x00874df4) = GP4_FIELD(uint16_t, esi, 0);
        GP4_GLOBAL(int32_t, 0x00874dfc) = static_cast<int16_t>(GP4_FIELD(uint16_t, esi, 2));
    }
    r->esi = esi + 4u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 4u));
}
GP4_IMPL(0x00555e6f, FUN_00555e6f, "regs(esi:ptr[0x10]:bytes) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
