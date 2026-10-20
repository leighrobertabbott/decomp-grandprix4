#include <gp4/gp4.h>

static inline uint32_t parityEvenFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// flags left by `cmp reg32, [mem32]`
static inline uint32_t comparisonFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | parityEvenFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// flags left by `test byte [mem], imm8` whose AND result is `result`
// (CF = OF = AF = 0)
static inline uint32_t testFlags8(uint32_t result) {
    return parityEvenFlag(result)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u);
}

// FUNCTION: GP4 0x00403a1e
void __cdecl FUN_00403a1e(gp4::Regs* r) {
    const uint32_t value = r->eax;
    const uint8_t* rec = (const uint8_t*)r->esi;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);

    uint32_t result = value < limit ? 0u : 1u;
    uint32_t flags;

    uint32_t t = rec[0xfe] & 0x40u;
    flags = testFlags8(t);
    if (t != 0u) {
        t = rec[0xfe] & 0x10u;
        flags = testFlags8(t);
        result = (t != 0u) ? 2u : 3u;
    } else {
        flags = comparisonFlags32(value, limit);
        if (value < limit) {
            t = rec[0x163] & 0x80u;
            flags = testFlags8(t);
            if (t != 0u) {
                t = rec[0x122] & 0x80u;
                flags = testFlags8(t);
                if (t != 0u) {
                    result = 6u;
                } else {
                    t = rec[0x163] & 0x40u;
                    flags = testFlags8(t);
                    if (t != 0u) {
                        t = rec[0x179] & 0x40u;
                        flags = testFlags8(t);
                        if (t != 0u) {
                            result = 7u;
                        } else {
                            t = rec[0x163] & 0x20u;
                            flags = testFlags8(t);
                            result = (t != 0u) ? 4u : 5u;
                        }
                    }
                }
            }
        }
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00403a1e, FUN_00403a1e, "regs(eax:u32, esi:ptr[0x188]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
