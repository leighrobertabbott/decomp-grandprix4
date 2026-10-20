#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Two scale factors (the pair stays at 0x34d / 0x357, or comes from the table at 0x0043cd01 for mode 1,
// or is lowered by a random nibble) applied to the value of 0x0043cdf8 and rescaled by their nominal
// 0x34d / 0x357. eax = first result, edx = second; ebx ecx edi esi ebp are preserved.
// The flags are those of the second `mul` (the `div` after it leaves them alone).
// FUNCTION: GP4 0x0043cd21
void __cdecl FUN_0043cd21(gp4::Regs* r) {
    const uint32_t index = r->eax;
    uint32_t first = 0x34du;
    uint32_t second = 0x357u;

    const uint16_t mode = GP4_GLOBAL(uint16_t, 0x007ad8a0);
    if (mode == 1u) {
        gp4::Regs table = *r;
        table.eax = index;
        gp4::call_regs(0x0043cd01, table);
        first = table.eax;
        second = table.edx;
    } else if (mode != 0u) {
        gp4::Regs noise = *r;
        gp4::call_regs(0x00401462, noise);
        const uint32_t drop = (noise.eax & 0xffu) >> 4;
        first -= drop;
        second -= drop;
    }

    gp4::Regs scale = *r;
    scale.eax = GP4_GLOBAL(uint16_t, 0x007ad89e);
    gp4::call_regs(0x0043cdf8, scale);
    const uint32_t value = scale.eax;

    const uint64_t firstProduct = static_cast<uint64_t>(value) * first;
    const uint32_t firstResult = static_cast<uint32_t>(firstProduct / 0x34du);

    const uint64_t secondProduct = static_cast<uint64_t>(value) * second;
    const uint32_t low = static_cast<uint32_t>(secondProduct);
    const uint32_t high = static_cast<uint32_t>(secondProduct >> 32);
    const uint32_t secondResult = static_cast<uint32_t>(secondProduct / 0x357u);

    // mul flags (pinned oracle model): CF = OF = high half nonzero, AF = 0, PF/ZF/SF from the low half
    const uint32_t flags = (high != 0u ? 0x801u : 0u)
        | parityFlag(low)
        | (low == 0u ? 0x40u : 0u)
        | ((low >> 24) & 0x80u);

    r->eax = firstResult;
    r->edx = secondResult;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043cd21, FUN_0043cd21, "regs(eax:u32[0..10]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
