#include <gp4/gp4.h>

// (value << shift) * factor, shifted right by 14 (imul + shrd eax, edx, 14): low 32 bits.
static inline uint32_t scaleDown14(int32_t value, int32_t factor) {
    const int64_t product = static_cast<int64_t>(value) * factor;
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x0040b99a
// pushal ... popal routine: rebuilds several scaled tables from the factor
// DAT_00622820 * DAT_00622778 / DAT_006227b0. Only the flags of the final
// "cmp ecx, 0xd" (equal: ZF and PF set) escape.
void __cdecl FUN_0040b99a(gp4::Regs* r) {
    int32_t rem;
    const int32_t factor = gp4::idiv64(static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00622820)) * GP4_GLOBAL(int32_t, 0x00622778),
                                       GP4_GLOBAL(int32_t, 0x006227b0), &rem);

    const int32_t* source = GP4_ARRAY(int32_t, 0x006225c0);
    uint32_t* dest760 = GP4_ARRAY(uint32_t, 0x00622760);
    uint32_t* dest688 = GP4_ARRAY(uint32_t, 0x00622688);
    for (uint32_t i = 3; i != 6; ++i) {
        const uint32_t scaled = scaleDown14(static_cast<int32_t>(static_cast<uint32_t>(source[i]) << 10), factor);
        dest760[i] = scaled;
        dest688[i] = scaled >> 2;
    }

    const uint8_t* pairsA = GP4_ARRAY(uint8_t, 0x006225f8);
    uint32_t* destA = GP4_ARRAY(uint32_t, 0x006226a0);
    for (uint32_t i = 0; i != 13; ++i) {
        uint32_t scaled = scaleDown14(static_cast<int32_t>(static_cast<uint32_t>(pairsA[i * 2]) << 8), factor);
        for (uint32_t bits = pairsA[i * 2 + 1]; bits != 0; bits >>= 1) {
            scaled >>= 1;
        }
        destA[i] = scaled;
    }

    const uint8_t* pairsB = GP4_ARRAY(uint8_t, 0x006225d8);
    uint32_t* destB = GP4_ARRAY(uint32_t, 0x006226e0);
    for (uint32_t i = 0; i != 13; ++i) {
        uint32_t scaled = scaleDown14(static_cast<int32_t>(static_cast<uint32_t>(pairsB[i * 2]) << 8), factor);
        for (uint32_t bits = pairsB[i * 2 + 1]; bits != 0; bits >>= 1) {
            scaled >>= 1;
        }
        destB[i] = scaled;
    }

    const int32_t divisor = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x0062d238)) * static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006227b0)));
    const int32_t left = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00622778)) * 1000u);
    const int32_t right = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062278c) << 8);
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(left) * right, divisor, &rem);

    uint32_t level = GP4_GLOBAL(uint32_t, 0x00622790) >> 4;
    if (static_cast<int32_t>(level) > 0xb) {
        level = 0xb;
    }
    const uint32_t gain = GP4_ARRAY(uint8_t, 0x0062280d)[level];
    const uint8_t* base = GP4_ARRAY(uint8_t, 0x006227fc);
    uint32_t* destC = GP4_ARRAY(uint32_t, 0x00622720);
    for (uint32_t i = 0; i != 13; ++i) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(base[i])) * quotient;
        const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 8);
        destC[i] = shifted * gain;
    }

    // popal restores the registers; the flags are those of the last "cmp ecx, 0xd" (0xd - 0xd)
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0040b99a, FUN_0040b99a, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0040b99a")
