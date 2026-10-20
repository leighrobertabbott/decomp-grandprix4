#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 8-bit `left + right` (ADD CL, AL).
static inline uint32_t addFlags8(uint32_t left, uint32_t right) {
    const uint32_t sum = (left & 0xffu) + (right & 0xffu);
    const uint32_t result = sum & 0xffu;
    return (sum > 0xffu ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// One of the four signed gains: (field * [0x601cc8]) >> 28, limited to -0x3f..0x3f (the magnitude is
// compared unsigned, so an overflowing negative value also ends at -0x3f). high = the high product word.
static inline uint32_t limitedGain(uint32_t field, uint32_t* high) {
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(field)) *
        static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00601cc8))));
    *high = static_cast<uint32_t>(product >> 32);
    uint32_t value = static_cast<uint32_t>(product >> 28);
    if (static_cast<int32_t>(value) < 0) {
        value = 0u - value;
        if (value >= 0x3fu) {
            value = 0x3fu;
        }
        value = 0u - value;
    } else if (value >= 0x3fu) {
        value = 0x3fu;
    }
    return value;
}

// Adds four signed limited gains (dwords at +0x39c, +0x3a0, +0x3a4, +0x3a8 times DAT_00601cc8 >> 28,
// each limited to -0x3f..0x3f) to the four bytes of the dword at +0x210 (byte i gets gain i, wrapping per
// byte). eax = the last gain, edx = the high product word of the last multiplication; the flags are those
// of the last `add cl, al` with CF / OF replaced by those of the final `ror ecx, 8`.
// FUNCTION: GP4 0x0043576f
void __cdecl FUN_0043576f(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t word = GP4_FIELD(uint32_t, body, 0x210);

    uint32_t value = 0;
    uint32_t high = 0;
    uint32_t flags = 0;
    for (uint32_t i = 0; i < 4u; ++i) {
        value = limitedGain(GP4_FIELD(uint32_t, body, 0x39c + i * 4u), &high);
        flags = addFlags8(word, value);
        word = (word & 0xffffff00u) | ((word + value) & 0xffu);
        word = (word >> 8) | (word << 24);   // ror ecx, 8
    }

    // ror by 8: CF = bit 31 of the result, OF = bit 31 xor bit 30; the other flags stay those of the add
    flags = (flags & 0xd4u) | ((word >> 31) & 1u) | ((((word >> 31) ^ (word >> 30)) & 1u) != 0u ? 0x800u : 0u);

    GP4_FIELD(uint32_t, body, 0x210) = word;
    r->eax = value;
    r->ecx = word;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043576f, FUN_0043576f, "regs(esi:ptr[0x3b0]:i32[-256..256]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
