#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clamps two 16-bit values into the ranges stored in the record esi: bx into [word +4, word +8]
// and ax into [word +6, word +0xa] (unsigned word compares; a clamp replaces the whole register
// with the zero-extended bound, otherwise the register is left alone). The flags are those of the
// last compare, `cmp ax, [esi + 0xa]`.
// FUNCTION: GP4 0x00539d21
void __cdecl FUN_00539d21(gp4::Regs* r) {
    uint32_t ebx = r->ebx;
    uint32_t eax = r->eax;
    if (static_cast<uint16_t>(ebx) < GP4_FIELD(uint16_t, r->esi, 4)) {
        ebx = GP4_FIELD(uint16_t, r->esi, 4);
    }
    if (static_cast<uint16_t>(ebx) > GP4_FIELD(uint16_t, r->esi, 8)) {
        ebx = GP4_FIELD(uint16_t, r->esi, 8);
    }
    if (static_cast<uint16_t>(eax) < GP4_FIELD(uint16_t, r->esi, 6)) {
        eax = GP4_FIELD(uint16_t, r->esi, 6);
    }
    const uint16_t upper = GP4_FIELD(uint16_t, r->esi, 0xa);
    const uint32_t arith = gp4::flags::sub16(eax, upper);
    if (static_cast<uint16_t>(eax) > upper) {
        eax = upper;
    }
    r->ebx = ebx;
    r->eax = eax;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00539d21, FUN_00539d21, "regs(eax:u32, ebx:u32, esi:ptr[0x10]:bytes) -> (eax, ebx, cf, pf, af, zf, sf, of)")
