#include <gp4/gp4.h>

// Fills the three-word pair at 0x603e38/0x603e3a/0x603e3c from two car records (esi = this car,
// ebx = the other one). dx = -|word +0x90| + word +0xb0 clamped to 0..[0x603e44]. When either
// car's word +0xea equals 0x20 the words are copied from 0x603e3e/40/42; otherwise they are
// dx + [0x603e32], dx + [0x603e34] and [0x603e36]. Only ax and dx change (upper halves stay);
// dx keeps the clamped value. Flags: a matching CMP (equal: ZF = PF = 1) on the copy path, the
// 16-bit ADD of dx + [0x603e34] on the other path.
static inline uint32_t parity_flag(uint32_t value) {
    uint32_t b = value & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return (b & 1u) ? 0u : 4u;   // PF set when the low byte has an even number of set bits
}

// FUNCTION: GP4 0x0042c155
void __cdecl FUN_0042c155(gp4::Regs* r) {
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    int16_t value = static_cast<int16_t>(GP4_FIELD(uint16_t, self, 0x90));
    if (value >= 0) value = static_cast<int16_t>(0 - value);      // NEG skipped for negatives, then the second NEG
    value = static_cast<int16_t>(static_cast<uint16_t>(value) + GP4_FIELD(uint16_t, self, 0xb0));
    if (value < 0) value = 0;
    if (value > static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00603e44))) value = static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00603e44));

    const uint16_t clamped = static_cast<uint16_t>(value);
    uint16_t ax;
    uint32_t flags;
    if (GP4_FIELD(uint16_t, other, 0xea) == 0x20u || GP4_FIELD(uint16_t, self, 0xea) == 0x20u) {
        GP4_GLOBAL(uint16_t, 0x00603e38) = GP4_GLOBAL(uint16_t, 0x00603e3e);
        GP4_GLOBAL(uint16_t, 0x00603e3a) = GP4_GLOBAL(uint16_t, 0x00603e40);
        ax = GP4_GLOBAL(uint16_t, 0x00603e42);
        GP4_GLOBAL(uint16_t, 0x00603e3c) = ax;
        flags = 0x44u;      // CMP of equal words: ZF = PF = 1, everything else clear
    } else {
        GP4_GLOBAL(uint16_t, 0x00603e38) = static_cast<uint16_t>(clamped + GP4_GLOBAL(uint16_t, 0x00603e32));
        const uint16_t addend = GP4_GLOBAL(uint16_t, 0x00603e34);
        const uint16_t sum = static_cast<uint16_t>(clamped + addend);
        GP4_GLOBAL(uint16_t, 0x00603e3a) = sum;
        ax = GP4_GLOBAL(uint16_t, 0x00603e36);
        GP4_GLOBAL(uint16_t, 0x00603e3c) = ax;
        flags = ((static_cast<uint32_t>(clamped) + addend) > 0xffffu ? 1u : 0u)
            | parity_flag(sum)
            | ((clamped ^ addend ^ sum) & 0x10u)
            | (sum == 0u ? 0x40u : 0u)
            | ((sum & 0x8000u) ? 0x80u : 0u)
            | (((~(clamped ^ addend) & (clamped ^ sum)) & 0x8000u) ? 0x800u : 0u);
    }

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->edx = (r->edx & 0xffff0000u) | clamped;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042c155, FUN_0042c155, "regs(eax:u32, edx:u32, ebx:rec[0x66fef0,0x41c,0..21], esi:rec[0x66fef0,0x41c,0..21]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0")
