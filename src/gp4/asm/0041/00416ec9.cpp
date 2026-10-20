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

// flags of or/xor/test (CF = OF = AF = 0)
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

// FUNCTION: GP4 0x00416ec9
// Computes a 0x4000-based scale factor at 0x0062fb54 from the state at 0x006380d8: it stays
// 0x4000 when the record's word at +0xee is zero or the two averaged sign tests disagree, or
// when the larger magnitude of +0x28 / +0x328 is below the threshold [0x62fb68]. The slope
// [0x62fb6c] and a blend of [0x62fb50] (kept in ebp) shape the result.
void __cdecl FUN_00416ec9(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(r->esi);
    uint8_t* const state = StateBase();
    GP4_GLOBAL(int32_t, 0x0062fb54) = 0x4000;

    const uint32_t mode = GP4_FIELD(uint16_t, rec, 0xee);
    r->eax = mode;
    if (mode == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags(mode);
        return;
    }
    r->ebx = 0x006380d8u;

    const int32_t avgA = static_cast<int32_t>(GP4_FIELD(uint32_t, state, 0x40) + GP4_FIELD(uint32_t, state, 0x340)) >> 1;
    const int32_t avgB = static_cast<int32_t>(GP4_FIELD(uint32_t, state, 0x1c) + GP4_FIELD(uint32_t, state, 0x31c)) >> 1;
    uint32_t eax = static_cast<uint32_t>(avgA);
    uint32_t edx = static_cast<uint32_t>(avgB) ^ eax;
    if (static_cast<int32_t>(edx) < 0) {
        r->eax = eax;
        r->edx = edx;
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags(edx);
        return;
    }

    uint32_t ebp = 0x4000u;
    if (GP4_GLOBAL(uint8_t, 0x00622424) == 0u && (GP4_FIELD(uint8_t, rec, 0xe5) & 4u) != 0u) {
        int32_t mag = static_cast<int32_t>(GP4_FIELD(uint32_t, state, 0x1c) + GP4_FIELD(uint32_t, state, 0x31c)) >> 1;
        mag = absValue(mag);
        if (mag > 0x80000) mag = 0x80000;
        if (mag < 0x40000) mag = 0x40000;
        const uint32_t scale = (static_cast<uint32_t>(mag) - 0x40000u) >> 4;
        const int32_t base = GP4_GLOBAL(int32_t, 0x0062fb50);
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(0x4000u - static_cast<uint32_t>(base)))
            * static_cast<int64_t>(static_cast<int32_t>(scale));
        edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        ebp = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + static_cast<uint32_t>(base);
        eax = ebp;
    }

    int32_t first = absValue(static_cast<int32_t>(GP4_FIELD(uint32_t, state, 0x28)));
    const int32_t second = absValue(static_cast<int32_t>(GP4_FIELD(uint32_t, state, 0x328)));
    edx = static_cast<uint32_t>(second);
    if (first < second) first = second;
    eax = static_cast<uint32_t>(first);
    r->ebp = ebp;
    const int32_t threshold = GP4_GLOBAL(int32_t, 0x0062fb68);
    if (first < threshold) {
        r->eax = eax;
        r->edx = edx;
        r->eflags = (r->eflags & ~0x8d5u) | cmpFlags(eax, static_cast<uint32_t>(threshold));
        return;
    }

    const int64_t first_product = static_cast<int64_t>(static_cast<int32_t>(eax - static_cast<uint32_t>(threshold)))
        * static_cast<int64_t>(static_cast<int32_t>(ebp));
    const int32_t narrowed = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(first_product) >> 14));
    const int64_t second_product = static_cast<int64_t>(narrowed) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062fb6c));
    edx = static_cast<uint32_t>(static_cast<uint64_t>(second_product) >> 32);
    int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(second_product) >> 14));
    if (scaled > 0x4000) scaled = 0x4000;
    if (scaled < 0) scaled = 0;
    const uint32_t negated = 0u - static_cast<uint32_t>(scaled);
    eax = negated + 0x4000u;
    GP4_GLOBAL(uint32_t, 0x0062fb54) = eax;

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(negated, 0x4000u);
}
GP4_IMPL(0x00416ec9, FUN_00416ec9, "regs(esi:ptr[0xf6]:layout=car_00416ec9, ebx:u32, ebp:u32, edx:u32) -> (eax, edx, ebx, ebp, cf, pf, af, zf, sf, of); cases=500; globals=fuzz:i32+layout=state_00416fce")
