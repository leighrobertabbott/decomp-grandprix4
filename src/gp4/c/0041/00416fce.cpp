#include <gp4/gp4.h>

// Keeps the compiler from folding state-relative offsets into absolute globals.
static __declspec(noinline) uint8_t* StateBase() { return GP4_ARRAY(uint8_t, 0x006380d8); }

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "xor reg, reg" (CF = OF = AF = 0)
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline int32_t absValue(int32_t v) {
    return v < 0 ? static_cast<int32_t>(0u - static_cast<uint32_t>(v)) : v;
}

// FUNCTION: GP4 0x00416fce
// Computes a 0x4000-based scale factor at 0x0062fb78 from two pairs of values in the
// 0x006380d8 state (+0x28 / +0x328 magnitudes against threshold [0x62fb70], slope
// [0x62fb74]); it stays 0x4000 when the two averaged sign tests disagree or the
// larger magnitude is below the threshold. ebx is left holding the state base.
void __cdecl FUN_00416fce(gp4::Regs* r) {
    uint8_t* const state = StateBase();
    GP4_GLOBAL(int32_t, 0x0062fb78) = 0x4000;

    const int32_t avgA = (GP4_FIELD(int32_t, state, 0x40) + GP4_FIELD(int32_t, state, 0x340)) >> 1;
    const int32_t avgB = (GP4_FIELD(int32_t, state, 0x1c) + GP4_FIELD(int32_t, state, 0x31c)) >> 1;
    uint32_t eax = static_cast<uint32_t>(avgA);
    uint32_t edx = static_cast<uint32_t>(avgB) ^ eax;
    uint32_t flags = logicFlags(edx);

    if (static_cast<int32_t>(edx) >= 0) {
        int32_t first = absValue(GP4_FIELD(int32_t, state, 0x28));
        const int32_t second = absValue(GP4_FIELD(int32_t, state, 0x328));
        edx = static_cast<uint32_t>(second);
        if (first < second) first = second;
        eax = static_cast<uint32_t>(first);
        const int32_t threshold = GP4_GLOBAL(int32_t, 0x0062fb70);
        flags = cmpFlags(eax, static_cast<uint32_t>(threshold));
        if (first >= threshold) {
            const int64_t product = static_cast<int64_t>(static_cast<int32_t>(eax - static_cast<uint32_t>(threshold)))
                * GP4_GLOBAL(int32_t, 0x0062fb74);
            edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
            int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
            if (scaled > 0x4000) scaled = 0x4000;
            if (scaled < 0) scaled = 0;
            const uint32_t negated = 0u - static_cast<uint32_t>(scaled);
            flags = addFlags(negated, 0x4000u);
            eax = negated + 0x4000u;
            GP4_GLOBAL(uint32_t, 0x0062fb78) = eax;
        }
    }
    r->eax = eax;
    r->edx = edx;
    r->ebx = 0x006380d8u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00416fce, FUN_00416fce, "regs(esi:ptr[0xed]:bytes) -> (eax, edx, ebx, cf, pf, af, zf, sf, of); globals=layout=state_00416fce")
