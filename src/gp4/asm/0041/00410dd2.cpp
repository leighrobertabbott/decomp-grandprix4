#include <gp4/gp4.h>

// Codes in a source byte that set the matching flag bit in the destination byte +0xe6.
static inline bool isFlaggedCode(uint8_t code) {
    return code == 6u || code == 7u || code == 0x20u || code == 0x22u || code == 0x23u ||
           code == 0x24u || code == 8u || code == 9u || code == 0x21u || code == 0x25u ||
           code == 0x26u || code == 0x27u;
}

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t low = value & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    return (low & 1u) == 0u ? 0x4u : 0u;
}

// FUNCTION: GP4 0x00410dd2
void __cdecl FUN_00410dd2(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    dst[0x17a] = src[4];
    dst[0x17b] = src[0xa];
    dst[0x1d2] = src[0xd];
    dst[0x1d3] = src[0xf];
    dst[0x41a] = src[0];

    // (int16)(byte << 8) * 0x2d82, keep bits 20..27 of the 32-bit product.
    const int32_t product = static_cast<int32_t>(static_cast<int16_t>(static_cast<uint16_t>(src[6] << 8))) * 0x2d82;
    dst[0x1d0] = static_cast<uint8_t>(static_cast<uint32_t>(product) >> 20);

    dst[0x1d1] = src[7];
    dst[0x178] = src[5];
    dst[0x173] = src[0x1e];

    dst[0xcf] = static_cast<uint8_t>(dst[0xcf] & 0xf7u);
    if ((src[1] & 1u) != 0u && GP4_GLOBAL(uint8_t, 0x007a97a0) != 0u) {
        dst[0xcf] = static_cast<uint8_t>(dst[0xcf] | 0x08u);
        if ((src[1] & 2u) != 0u)
            dst[0xcf] = static_cast<uint8_t>(dst[0xcf] | 0x04u);
    }
    dst[0xcf] = static_cast<uint8_t>(dst[0xcf] & 0xdfu);
    if (src[0x10] != src[0x11])
        dst[0xcf] = static_cast<uint8_t>(dst[0xcf] | 0x20u);

    uint8_t flags = 0;
    if (isFlaggedCode(src[2])) flags |= 0x80u;
    if (src[3] != 0u) flags |= 0x08u;

    dst[0x78] = static_cast<uint8_t>(dst[0x78] | 0x20u);
    if ((flags & 0x08u) != 0u && src[5] == 0u)
        dst[0x78] = static_cast<uint8_t>(dst[0x78] & 0xdfu);

    if (isFlaggedCode(src[8])) flags |= 0x40u;
    if (src[9] != 0u) flags |= 0x04u;
    if (isFlaggedCode(src[0xb])) flags |= 0x20u;
    if (src[0xc] != 0u) flags |= 0x02u;

    const uint8_t last = src[0xe];
    uint32_t eflags;
    if (isFlaggedCode(last)) {
        flags |= 0x11u;
        // or byte [esi+0xe6], 1: CF = OF = AF = 0.
        eflags = parityFlag(flags) | ((flags & 0x80u) != 0u ? 0x80u : 0u);
    } else {
        // cmp bl, 0x27 (not equal).
        const uint8_t diff = static_cast<uint8_t>(last - 0x27u);
        eflags = (last < 0x27u ? 0x1u : 0u)
            | parityFlag(diff)
            | (((last ^ 0x27u ^ diff) & 0x10u) != 0u ? 0x10u : 0u)
            | (diff == 0u ? 0x40u : 0u)
            | ((diff & 0x80u) != 0u ? 0x80u : 0u)
            | ((((last ^ 0x27u) & (last ^ diff)) & 0x80u) != 0u ? 0x800u : 0u);
    }
    dst[0xe6] = flags;

    r->eflags = (r->eflags & ~0x8d5u) | eflags;
}
GP4_IMPL(0x00410dd2, FUN_00410dd2, "regs(ebx:ptr[0x26]:bytes, esi:ptr[0x422]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=flag_007a97a0; cases=200")
