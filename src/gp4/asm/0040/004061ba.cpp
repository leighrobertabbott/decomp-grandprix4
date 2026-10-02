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

// FUNCTION: GP4 0x004061ba
void __cdecl FUN_004061ba(gp4::Regs* r) {
    const uint32_t original = r->edi;
    const uint8_t* body = reinterpret_cast<const uint8_t*>(r->esi);
    uint32_t selector = GP4_GLOBAL(uint32_t, 0x00891504);
    uint32_t flags = comparisonFlags32(original, selector);
    if (original == selector) {
        r->edi = GP4_GLOBAL(uint32_t, 0x00866ecc);
    } else {
        selector = GP4_GLOBAL(uint32_t, 0x00891500);
        flags = comparisonFlags32(original, selector);
        if (original == selector) {
            r->edi = GP4_GLOBAL(uint32_t, 0x00866f00);
        } else {
            selector = GP4_GLOBAL(uint32_t, 0x00866f04);
            flags = comparisonFlags32(original, selector);
            if (original == selector) {
                const bool set = (body[0x71] & 0x80u) != 0u;
                flags = set ? 0x80u : 0x44u;
                if (set) r->edi = GP4_GLOBAL(uint32_t, 0x00866ef8);
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004061ba, FUN_004061ba, "regs(edi:u32[0..3], esi:ptr[0x79]:bytes) -> (edi, cf, pf, af, zf, sf, of); globals=layout=remap_00406187")
