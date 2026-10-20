#include <gp4/gp4.h>

// 16-bit packed position of record `p` made of two words: (word hi >> 8) + (word lo << 6).
static inline uint16_t packWords(uint8_t* p, uint32_t hiOff, uint32_t loOff) {
    const int16_t hi = GP4_FIELD(const int16_t, p, hiOff);
    const uint16_t lo = GP4_FIELD(const uint16_t, p, loOff);
    return static_cast<uint16_t>(static_cast<uint16_t>(hi >> 8) + static_cast<uint16_t>(lo << 6));
}

// FUNCTION: GP4 0x0040dc8a
// ax = (value of the record in ebx) - word [esi+0xe2], where the value of ebx is its word +0xe2 or,
// depending on bit 2 of [esi+0x21d]/bit 1 of [esi+0x9c] against bit 2 of [ebx+0x21d], the packed
// pair (+0x14e >> 8) + (+0x14c << 6) or (+0x14a >> 8) + (+0x148 << 6). The upper half of eax is kept,
// ecx and edx are restored, the flags are those of the 16-bit sub. (The word the original computes
// from [esi+0x14c/0x14e] when [esi+0x9c] bit 1 is set is overwritten before use.)
void __cdecl FUN_0040dc8a(gp4::Regs* r) {
    uint8_t* esi = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* ebx = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    const uint16_t reference = GP4_FIELD(const uint16_t, esi, 0xe2);
    uint16_t value;
    if ((GP4_FIELD(const uint8_t, esi, 0x21d) & 4u) != 0u || (GP4_FIELD(const uint8_t, esi, 0x9c) & 2u) != 0u) {
        value = GP4_FIELD(const uint16_t, ebx, 0xe2);
        if ((GP4_FIELD(const uint8_t, ebx, 0x21d) & 4u) == 0u) {
            value = packWords(ebx, 0x14e, 0x14c);
        }
    } else {
        value = GP4_FIELD(const uint16_t, ebx, 0xe2);
        if ((GP4_FIELD(const uint8_t, ebx, 0x21d) & 4u) != 0u) {
            value = packWords(ebx, 0x14a, 0x148);
        }
    }

    const uint32_t left = value;
    const uint32_t right = reference;
    const uint32_t result = (left - right) & 0xffffu;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = (left < right ? 0x01u : 0u)
        | ((~parity & 1u) << 2)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);

    r->eax = (r->eax & 0xffff0000u) | result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040dc8a, FUN_0040dc8a, "regs(eax:u32, ebx:ptr[0x225]:bytes, esi:ptr[0x225]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
