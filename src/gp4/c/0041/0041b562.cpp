#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of test/xor/and/or: CF=OF=AF=0
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// flags of cmp left, right (and neg value == cmp 0, value)
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

// FUNCTION: GP4 0x0041b562
void __cdecl FUN_0041b562(gp4::Regs* r) {
    uint8_t* rec = (uint8_t*)r->esi;
    const uint32_t a = GP4_GLOBAL(uint32_t, 0x00638d20);
    uint32_t flags;
    if (a == 0u) {
        flags = logicFlags(a);
    } else {
        const uint32_t b = GP4_FIELD(uint32_t, rec, 0x3c);
        uint32_t x = b ^ a;
        r->edx = x;
        flags = logicFlags(x);
        if ((x & 0x80000000u) != 0u) {
            x = a ^ r->eax;
            r->edx = x;
            flags = logicFlags(x);
            if ((x & 0x80000000u) != 0u) {
                uint32_t m = absWrap(b);
                r->edx = m;
                flags = subFlags(m, GP4_GLOBAL(uint32_t, 0x00603530));
                if (!((int32_t)m < (int32_t)GP4_GLOBAL(uint32_t, 0x00603530))) {
                    m = absWrap(a);
                    r->edx = m;
                    flags = subFlags(m, GP4_GLOBAL(uint32_t, 0x00603534));
                    if (!((int32_t)m < (int32_t)GP4_GLOBAL(uint32_t, 0x00603534))) {
                        r->eax = 0u;
                    }
                }
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041b562, FUN_0041b562, "regs(eax:i32, esi:ptr[0x44]:i32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
