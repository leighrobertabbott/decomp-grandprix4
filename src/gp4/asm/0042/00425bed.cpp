#include <gp4/gp4.h>

// Interpolates between the two words at ebx+4 and ebx+6 by a Q14 fraction t clamped to 0..0x4000:
// result = word4 + (((word6 - word4) * t) >> 14). The fraction is ((target << 14) / span) where span
// is the sum of two extents (words at +0x60/+0x62 of the record at ebx+8, or esi+0x11c/+0x11e
// plus 0x100 each when that pointer is null) and target is 0x100 or 0x200 (esi+0x163 bit 5 set,
// chosen by the sign of word4) or span - 0x100 / span - 0x200 (chosen by the sign of word6).
// ecx and ebp are saved and restored; the 64-bit product leaves its high dword in edx and the final
// ADD of word4 leaves the flags.
static inline uint32_t parity_flag(uint32_t value) {
    uint32_t b = value & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return (b & 1u) ? 0u : 4u;   // PF set when the low byte has an even number of set bits
}

// FUNCTION: GP4 0x00425bed
void __cdecl FUN_00425bed(gp4::Regs* r) {
    uint8_t* block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* state = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint8_t* extents = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, block, 0x08)));
    uint32_t low_extent;    // ecx (negated)
    uint32_t high_extent;   // ebp
    if (extents != nullptr) {
        high_extent = GP4_FIELD(uint16_t, extents, 0x62);
        low_extent = 0u - static_cast<uint32_t>(GP4_FIELD(uint16_t, extents, 0x60));
    } else {
        low_extent = 0u - (GP4_FIELD(uint32_t, state, 0x11c) + 0x100u);
        high_extent = GP4_FIELD(uint32_t, state, 0x11e) + 0x100u;
    }
    const uint32_t span = high_extent - low_extent;

    uint32_t target;
    if (!(GP4_FIELD(uint8_t, state, 0x163) & 0x20u)) {
        target = span - 0x100u;
        if (GP4_FIELD(int16_t, block, 0x06) < 0) target -= 0x100u;
    } else {
        target = 0x100u;
        if (GP4_FIELD(int16_t, block, 0x04) >= 0) target += 0x100u;
    }

    int32_t remainder;
    int32_t fraction = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(target << 14)),
                                   static_cast<int32_t>(span), &remainder);
    if (fraction < 0) fraction = 0;
    if (fraction > 0x4000) fraction = 0x4000;

    const int32_t start = GP4_FIELD(int16_t, block, 0x04);
    const int32_t delta = static_cast<int32_t>(static_cast<uint32_t>(GP4_FIELD(int16_t, block, 0x06)) - static_cast<uint32_t>(start));
    const int64_t product = static_cast<int64_t>(delta) * fraction;
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t addend = static_cast<uint32_t>(start);
    const uint32_t result = scaled + addend;

    const uint32_t flags = ((static_cast<uint64_t>(scaled) + addend) >> 32 ? 1u : 0u)
        | parity_flag(result)
        | (((scaled ^ addend ^ result) & 0x10u))
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) ? 0x80u : 0u)
        | (((((scaled ^ result) & (addend ^ result)) >> 31) & 1u) << 11);

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00425bed, FUN_00425bed, "regs(ebx:ptr[0x10]:layout=blk_00425bed, esi:ptr[0x170]:layout=car_00425bed) -> (eax, edx, cf, pf, af, zf, sf, of)")
