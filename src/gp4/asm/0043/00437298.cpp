#include <gp4/gp4.h>

static inline uint32_t comparisonFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (static_cast<int32_t>(left) < static_cast<int32_t>(right) ? 1u : 0u);
}

// FUNCTION: GP4 0x00437298
void __cdecl FUN_00437298(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(r->esi);
    if ((body[0xe5] & 1u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }
    const uint32_t threshold = GP4_GLOBAL(uint32_t, 0x00629e5c);
    r->eax = threshold;
    uint32_t value = GP4_FIELD(uint32_t, body, 0x2fc);
    if (static_cast<int32_t>(threshold) < static_cast<int32_t>(value)) {
        r->eflags = (r->eflags & ~0x8d5u) | comparisonFlags32(threshold, value);
        return;
    }
    value = GP4_FIELD(uint32_t, body, 0x300);
    if (static_cast<int32_t>(threshold) < static_cast<int32_t>(value)) {
        r->eflags = (r->eflags & ~0x8d5u) | comparisonFlags32(threshold, value);
        return;
    }
    value = GP4_FIELD(uint32_t, body, 0x304);
    if (static_cast<int32_t>(threshold) < static_cast<int32_t>(value)) {
        r->eflags = (r->eflags & ~0x8d5u) | comparisonFlags32(threshold, value);
        return;
    }
    value = GP4_FIELD(uint32_t, body, 0x308);
    r->eflags = (r->eflags & ~0x8d5u) | comparisonFlags32(threshold, value);
}
GP4_IMPL(0x00437298, FUN_00437298, "regs(esi:ptr[0x310]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

