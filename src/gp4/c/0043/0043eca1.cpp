#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of test/xor: CF=OF=AF=0
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// flags of cmp left, right
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

static inline uint32_t absWrap(uint32_t value) {
    return (value & 0x80000000u) != 0u ? 0u - value : value;
}

// FUNCTION: GP4 0x0043eca1
void __cdecl FUN_0043eca1(gp4::Regs* r) {
    const uint32_t a = GP4_GLOBAL(uint32_t, 0x0062695c);
    uint32_t flags = logicFlags(a);
    uint8_t result;
    if (a == 0u) {
        result = 0xffu;
    } else {
        const uint32_t b = GP4_GLOBAL(uint32_t, 0x00626958);
        flags = logicFlags(b);
        if (b == 0u) {
            result = 0u;
        } else {
            flags = logicFlags(a ^ b);
            if ((a ^ b) & 0x80000000u) {
                result = 0u;
            } else {
                const uint32_t ma = absWrap(a);
                const uint32_t mb = absWrap(b);
                flags = subFlags(ma, mb);
                result = ((int32_t)ma > (int32_t)mb) ? 0u : 0xffu;
            }
        }
    }
    GP4_GLOBAL(uint8_t, 0x0062698c) = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043eca1, FUN_0043eca1, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
