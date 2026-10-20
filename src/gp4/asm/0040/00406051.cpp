#include <gp4/gp4.h>

static inline uint32_t comparisonFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Remaps the record pointer in edi: below DAT_00891500 the pointer equal to DAT_00866ecc becomes
// the array base 0x008943a0; at or above it the pointer equal to DAT_00866ef8 becomes
// DAT_00866f04. The flags are those of the last "cmp edi, ..." executed.
// FUNCTION: GP4 0x00406051
void __cdecl FUN_00406051(gp4::Regs* r) {
    const uint32_t original = r->edi;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
    uint32_t flags = comparisonFlags32(original, limit);
    if (original < limit) {
        const uint32_t selector = GP4_GLOBAL(uint32_t, 0x00866ecc);
        flags = comparisonFlags32(original, selector);
        if (original == selector) {
            r->edi = 0x008943a0u;
        }
    } else {
        const uint32_t selector = GP4_GLOBAL(uint32_t, 0x00866ef8);
        flags = comparisonFlags32(original, selector);
        if (original == selector) {
            r->edi = GP4_GLOBAL(uint32_t, 0x00866f04);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00406051, FUN_00406051, "regs(edi:u32[0..3]) -> (edi, cf, pf, af, zf, sf, of); globals=layout=remap_00406187")
