#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares eax with the blend (ebx * 0x67 + ecx * 0x99) >> 8 of ebx and ecx (weights sum to 256,
// 32-bit wrap, arithmetic shift). The order of the comparison follows the saved `cmp ecx, ebx`
// (restored with popfd): ecx >= ebx compares eax with the blend, otherwise the blend with eax. CF = 1
// (stc) when the first operand is >= the second (signed), else 0 (clc); the other flags are those of
// that compare. eax, ebx and ecx are pushed and popped.
// FUNCTION: GP4 0x00534538
void __cdecl FUN_00534538(gp4::Regs* r) {
    const uint32_t value = r->eax;
    const uint32_t first = r->ebx;
    const uint32_t second = r->ecx;
    const uint32_t blend =
        static_cast<uint32_t>(static_cast<int32_t>(first * 0x67u + second * 0x99u) >> 8);
    uint32_t flags;
    uint32_t carry;
    if (static_cast<int32_t>(second) >= static_cast<int32_t>(first)) {
        flags = gp4::flags::sub32(value, blend);
        carry = (static_cast<int32_t>(value) >= static_cast<int32_t>(blend)) ? 1u : 0u;
    } else {
        flags = gp4::flags::sub32(blend, value);
        carry = (static_cast<int32_t>(blend) >= static_cast<int32_t>(value)) ? 1u : 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, (flags & ~1u) | carry);
}
GP4_IMPL(0x00534538, FUN_00534538, "regs(eax:u32, ebx:u32, ecx:u32) -> (cf, pf, af, zf, sf, of)")
