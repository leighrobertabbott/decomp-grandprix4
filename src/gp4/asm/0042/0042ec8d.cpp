#include <gp4/gp4.h>

static inline uint32_t comparisonFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (static_cast<int16_t>(left) < static_cast<int16_t>(right) ? 1u : 0u);
}

// FUNCTION: GP4 0x0042ec8d
void __cdecl FUN_0042ec8d(gp4::Regs* r) {
    uint8_t* first = reinterpret_cast<uint8_t*>(r->esi);
    uint8_t* second = reinterpret_cast<uint8_t*>(r->ebx);
    const uint16_t difference = static_cast<uint16_t>(GP4_FIELD(uint16_t, first, 0x9a) - GP4_FIELD(uint16_t, second, 0x9a));
    const int64_t product = static_cast<int64_t>(static_cast<int16_t>(difference)) * GP4_GLOBAL(int32_t, 0x00603bcc);
    const uint64_t productBits = static_cast<uint64_t>(product);
    const uint32_t scaled = static_cast<uint32_t>(productBits >> 14);
    const uint16_t threshold = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00603e70) - GP4_GLOBAL(uint16_t, 0x00603ba0));
    r->eax = scaled;
    r->edx = (static_cast<uint32_t>(productBits >> 32) & 0xffff0000u) | threshold;
    r->eflags = (r->eflags & ~0x8d5u) | comparisonFlags16(static_cast<uint16_t>(scaled), threshold);
}
GP4_IMPL(0x0042ec8d, FUN_0042ec8d, "regs(ebx:ptr[0xa2]:bytes, esi:ptr[0xa2]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=compare_0042ec8d")

