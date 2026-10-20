#include <gp4/gp4.h>

// 16-bit absolute value as the original computes it: "test ax, ax / jns / neg ax".
static inline uint16_t Abs16(uint16_t value) {
    return static_cast<int16_t>(value) < 0 ? static_cast<uint16_t>(0u - value) : value;
}

// Table word selected by a 16-bit angle: (angle >> 2) & 0xfffe is a byte offset
// into the int16 table at 0x007c5c5c (sign-extended by movsx).
static inline int32_t TableWord(uint16_t angle) {
    const uint32_t byteOffset = static_cast<uint32_t>(static_cast<uint16_t>(angle >> 2)) & 0xfffeu;
    return GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), byteOffset);
}

// FUNCTION: GP4 0x0040c3c8
void __cdecl FUN_0040c3c8(gp4::Regs* r) {
    const uint64_t wide = static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x00603854)) * GP4_GLOBAL(uint32_t, 0x0062250c);
    const uint32_t scaled = static_cast<uint32_t>(wide >> 8);

    const uint16_t angle = GP4_GLOBAL(uint16_t, 0x00622514);

    const uint16_t complement = Abs16(static_cast<uint16_t>(0x4000u - angle));
    const int64_t first = static_cast<int64_t>(TableWord(complement)) * static_cast<int32_t>(scaled);
    GP4_GLOBAL(uint32_t, 0x0063b820) = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14);

    const int64_t second = static_cast<int64_t>(TableWord(Abs16(angle))) * static_cast<int32_t>(scaled);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14);
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 13);
    GP4_GLOBAL(uint32_t, 0x0063b824) = result;

    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    // Flags of the final "shrd eax, edx, 14" (pinned oracle model: AF = 0,
    // CF = bit 13 of the product, OF = bit31(product>>13 ^ product>>14)).
    const uint32_t flags = static_cast<uint32_t>((static_cast<uint64_t>(second) >> 13) & 1u)
        | ((~parity & 1u) << 2)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((previous ^ result) >> 31) << 11);

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
    r->ebp = scaled;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040c3c8, FUN_0040c3c8, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
