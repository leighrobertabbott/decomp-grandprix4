#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of cmp left, right
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Snaps the 24.8 fixed-point value in eax to a band boundary. A blended threshold
// v = ((a - d) * DAT_006227a0 >> 14) + d (d = 0x63b8a0 - 0x63b8a8, a = 0x63b8a4 - 0x63b8ac)
// decides, for bucket (eax + 0x80) >> 8 == 2: v < 0x34 -> 0x100, v >= 0x59 -> 0x300; for bucket
// 4: v < 0x79 -> 0x300, v >= 0x99 -> 0x500. Everything else (other buckets, v inside a band)
// returns eax unchanged. ebp and edx are saved and restored; the flags are those of the last cmp.
// FUNCTION: GP4 0x0043c953
void __cdecl FUN_0043c953(gp4::Regs* r) {
    const uint32_t input = r->eax;

    const uint32_t d = GP4_GLOBAL(uint32_t, 0x0063b8a0) - GP4_GLOBAL(uint32_t, 0x0063b8a8);
    const uint32_t a = GP4_GLOBAL(uint32_t, 0x0063b8a4) - GP4_GLOBAL(uint32_t, 0x0063b8ac);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a - d)) * GP4_GLOBAL(int32_t, 0x006227a0);
    const uint32_t blend = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + d;
    const int32_t value = static_cast<int32_t>(blend);

    const uint32_t bucket = (input + 0x80u) >> 8;
    uint32_t result = input;
    uint32_t flags;
    if (bucket == 2u) {
        if (value < 0x34) {
            flags = cmpFlags(blend, 0x34u);
            result = 0x100u;
        } else {
            flags = cmpFlags(blend, 0x59u);
            if (value >= 0x59) {
                result = 0x300u;
            }
        }
    } else if (bucket == 4u) {
        if (value < 0x79) {
            flags = cmpFlags(blend, 0x79u);
            result = 0x300u;
        } else {
            flags = cmpFlags(blend, 0x99u);
            if (value >= 0x99) {
                result = 0x500u;
            }
        }
    } else {
        flags = cmpFlags(bucket, 4u);
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043c953, FUN_0043c953, "regs(eax:u32[0..0x600]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0043c1ba")
