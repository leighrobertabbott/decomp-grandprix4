#include <gp4/gp4.h>

// Sum of the table bytes selected by the set bits of `mask`: bit j picks tab[lane + 4*j].
static inline uint32_t sumSelected(uint32_t mask, const uint8_t* tab, uint32_t lane) {
    uint32_t sum = 0;
    uint32_t off = lane;
    while (mask != 0) {
        if (mask & 1u) sum += tab[off];
        mask >>= 1;
        off += 4;
    }
    return sum;
}

// FUNCTION: GP4 0x004372ca
// pushal/popal routine: for each of the four lanes k, sums the bytes of two 32-entry x 4-byte tables
// (0x00629be8 and 0x00629c28) selected by the bit masks in the record at edi (+0x18, +0x24 and
// +0x10 masked by 0x00629be4 / 0x00629be0), scales the two differences by the 18.13 factors eax and
// edx, clamps the sum to 0..0x7f and stores it at esi+0x1a8+k. Finally esi+0x1c4 = 0x20.
// Registers are all restored; the flags left behind are those of the loop's final `cmp ebx, 4`.
void __cdecl FUN_004372ca(gp4::Regs* r) {
    const int32_t scaleA = (int32_t)r->eax;
    const int32_t scaleB = (int32_t)r->edx;
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* const out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(int32_t, 0x00629da0) = scaleA;
    GP4_GLOBAL(int32_t, 0x00629da4) = scaleB;

    for (uint32_t lane = 0; lane != 4; ++lane) {
        const uint32_t sumB0 = sumSelected(GP4_FIELD(uint32_t, rec, 0x18), GP4_ARRAY(const uint8_t, 0x00629be8), lane);
        const uint32_t sumAC = sumSelected(GP4_FIELD(uint32_t, rec, 0x10) & GP4_GLOBAL(uint32_t, 0x00629be4),
                                           GP4_ARRAY(const uint8_t, 0x00629c28), lane);
        const uint32_t sumB4 = sumSelected(GP4_FIELD(uint32_t, rec, 0x24), GP4_ARRAY(const uint8_t, 0x00629be8), lane);
        const uint32_t sumA8 = sumSelected(GP4_FIELD(uint32_t, rec, 0x10) & GP4_GLOBAL(uint32_t, 0x00629be0),
                                           GP4_ARRAY(const uint8_t, 0x00629c28), lane);

        GP4_GLOBAL(uint32_t, 0x00629db0) = sumB0;
        GP4_GLOBAL(uint32_t, 0x00629db4) = sumB4;
        GP4_GLOBAL(uint32_t, 0x00629da8) = sumA8;
        GP4_GLOBAL(uint32_t, 0x00629dac) = sumAC;

        const int64_t prodA = (int64_t)(int32_t)(sumAC - sumA8) * (int64_t)scaleA;
        const uint32_t partA = (uint32_t)(prodA >> 13);
        const int64_t prodB = (int64_t)(int32_t)(sumB0 - sumB4) * (int64_t)scaleB;
        const uint32_t partB = (uint32_t)(prodB >> 13);

        int32_t v = (int32_t)(partB + partA);
        if (v < 0) v = 0;
        if (v > 0x7f) v = 0x7f;
        out[0x1a8 + lane] = (uint8_t)v;
    }
    out[0x1c4] = 0x20;

    // Last flag writer: cmp ebx, 4 with ebx == 4 (popal restores no flags).
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004372ca, FUN_004372ca, "regs(eax:u32, edx:u32, edi:ptr[0x40]:bytes, esi:ptr[0x1d0]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
