#include <gp4/gp4.h>
#include <gp4/flags.h>

// al &= 0x1f; al < 0x1a ? al + 0x61 ('a'..'z') : al + 0x16 ('0'..'9' range after 0x1a).
// Only al changes; the flags are those of the final `add al, imm8`.
// FUNCTION: GP4 0x0052f4f9
void __cdecl FUN_0052f4f9(gp4::Regs* r) {
    const uint32_t v = r->eax & 0x1fu;
    const uint32_t addend = (v > 0x19u) ? 0x16u : 0x61u;
    const uint32_t flags = gp4::flags::add8(v, addend);
    r->eax = (r->eax & 0xffffff00u) | ((v + addend) & 0xffu);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052f4f9, FUN_0052f4f9, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
