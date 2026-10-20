#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00441166
// Per-car tyre-set / level choice for the car at esi (byte +0x7c = car number, +0x7f = level / tyre set).
// k = ((+0x7c) - 1) & 0x3f clamped to 0x15 selects the slot of the flag bytes 0x0066d920 and the stage
// bytes 0x0066d936. First time (flag bit 0 clear): set the flag; a level below 2 stores
// table6f2cc4[level] + table6f2d0c[0x51fb4f()], otherwise 0x4411f7 picks the stage from the level's base.
// Afterwards the stored stage is mapped through the byte table 0x006f2c3e back into +0x7f.
// Outputs: eax = k, edx = stage / byte, flags of the last arithmetic instruction.
void __cdecl FUN_00441166(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const taken = GP4_ARRAY(uint8_t, 0x0066d920);
    uint8_t* const stage = GP4_ARRAY(uint8_t, 0x0066d936);

    const uint32_t rawIndex = (static_cast<uint32_t>(car[0x7c]) - 1u) & 0x3fu;
    uint32_t index = rawIndex;
    if (static_cast<int32_t>(index) > 0x15) {
        index = 0x15;
    }

    uint32_t edx;
    uint32_t flags;
    if ((taken[index] & 1u) == 0u) {
        taken[index] |= 1u;
        const uint32_t level = car[0x7f];
        if (static_cast<int32_t>(level) < 2) {
            const uint32_t base = GP4_ARRAY(uint32_t, 0x006f2cc4)[level];
            const int32_t kind = gp4::cdecl_<0x0051fb4f, int32_t>();
            const uint32_t step = GP4_ARRAY(uint32_t, 0x006f2d0c)[kind];
            edx = base + step;
            flags = addFlags32(base, step);
        } else {
            const uint32_t base = GP4_ARRAY(uint32_t, 0x006f2cc4)[level];
            gp4::Regs call = *r;
            call.st_in = 0;
            call.st_out = 0;
            call.eax = index;
            call.edx = base;
            gp4::call_regs(0x004411f7, call);
            edx = call.edx;
            flags = call.eflags & 0x8d5u;
        }
        stage[index] = static_cast<uint8_t>(edx);
    } else {
        edx = GP4_ARRAY(uint8_t, 0x006f2c3e)[stage[index]];
        car[0x7f] = static_cast<uint8_t>(edx);
        flags = cmpFlags32(rawIndex, 0x15u);
    }

    r->eax = index;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00441166, FUN_00441166, "regs(esi:ptr[0x87]:layout=lvl_00431d1f) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0040860a+state_00405f77+state_004059ef")
