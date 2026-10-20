#include <gp4/gp4.h>

static inline uint32_t parityEvenFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `shl eax, 8` (pinned oracle model: CF = last bit out, AF = 0,
// OF = bit31 of (before << 7) ^ result).
static inline uint32_t shl8Flags(uint32_t before) {
    const uint32_t result = before << 8;
    return ((before >> 24) & 1u)
        | parityEvenFlag(result)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((before << 7) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Car record in esi: speed-like word +0x9a is scaled by DAT_00601b10 (Q14). With bit 1 of +0x8d
// and bit 7 of +0x26b both set the result is clamped to [DAT_00601b00 .. DAT_00601b04] (edx = high
// half of the product); otherwise it is clamped with min DAT_00601b0c, then max DAT_00601b08, and
// raised to |word +0x142 - word +0xe| (capped at DAT_00601b2c) * DAT_00601b30 >> 14 (kept in edx).
// The result is shifted left by 8.
// FUNCTION: GP4 0x004292a3
void __cdecl FUN_004292a3(gp4::Regs* r) {
    uint8_t* const self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const int32_t scale = GP4_GLOBAL(int32_t, 0x00601b10);
    const int32_t input = GP4_FIELD(int16_t, self, 0x9a);
    const int64_t product = static_cast<int64_t>(scale) * static_cast<int64_t>(input);
    int32_t value = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
    uint32_t edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);

    if ((self[0x8d] & 2u) != 0u && (self[0x26b] & 0x80u) != 0u) {
        if (value < GP4_GLOBAL(int32_t, 0x00601b00)) value = GP4_GLOBAL(int32_t, 0x00601b00);
        if (value > GP4_GLOBAL(int32_t, 0x00601b04)) value = GP4_GLOBAL(int32_t, 0x00601b04);
    } else {
        if (value > GP4_GLOBAL(int32_t, 0x00601b0c)) value = GP4_GLOBAL(int32_t, 0x00601b0c);
        if (value < GP4_GLOBAL(int32_t, 0x00601b08)) value = GP4_GLOBAL(int32_t, 0x00601b08);
        int32_t difference = static_cast<int16_t>(static_cast<uint16_t>(GP4_FIELD(uint16_t, self, 0x142) - GP4_FIELD(uint16_t, self, 0x0e)));
        if (difference < 0) difference = -difference;
        if (difference > GP4_GLOBAL(int32_t, 0x00601b2c)) difference = GP4_GLOBAL(int32_t, 0x00601b2c);
        const int64_t product2 = static_cast<int64_t>(difference) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601b30));
        edx = static_cast<uint32_t>(static_cast<uint64_t>(product2) >> 14);
        if (value < static_cast<int32_t>(edx)) value = static_cast<int32_t>(edx);
    }

    r->eax = static_cast<uint32_t>(value) << 8;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | shl8Flags(static_cast<uint32_t>(value));
}
GP4_IMPL(0x004292a3, FUN_004292a3, "regs(esi:ptr[0x273]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
