#include <gp4/gp4.h>

// (a - b) clamped at 0, times scale (signed 64-bit), shifted right by 14 (imul + shrd eax, edx, 14)
static inline uint32_t scaledDifference(uint32_t a, uint32_t b, int32_t scale) {
    int32_t diff = static_cast<int32_t>(a - b);
    if (diff < 0) {
        diff = 0;
    }
    const int64_t product = static_cast<int64_t>(diff) * static_cast<int64_t>(scale);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x0043c1ba
void __cdecl FUN_0043c1ba(gp4::Regs* r) {
    // popal restores every register comes back, only the flags of the final "cmp ecx, 6" remain
    const uint32_t first = scaledDifference(GP4_GLOBAL(uint32_t, 0x0063b8a0), GP4_GLOBAL(uint32_t, 0x0063b8a8),
                                            GP4_GLOBAL(int32_t, 0x006227a4));
    const uint32_t second = scaledDifference(GP4_GLOBAL(uint32_t, 0x0063b8a4), GP4_GLOBAL(uint32_t, 0x0063b8ac),
                                             GP4_GLOBAL(int32_t, 0x006227a8));
    int32_t position = static_cast<int32_t>(second + first);
    if (position < 0) {
        position = 0;
    }
    if (position > 0xb0) {
        position = 0xb0;
    }

    uint32_t* indices = GP4_ARRAY(uint32_t, 0x0062d1d8);
    uint32_t* results = GP4_ARRAY(uint32_t, 0x006227c4);
    for (uint32_t i = 0; i != 6; ++i) {
        gp4::Regs call = *r;
        call.ebx = indices[i];
        call.edx = static_cast<uint32_t>(position);
        call.ecx = i;
        gp4::call_regs(0x0041efb5, call);
        results[i] = call.eax;
    }

    // cmp ecx, 6 with ecx == 6: ZF and PF set, everything else clear
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043c1ba, FUN_0043c1ba, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0043c1ba")
