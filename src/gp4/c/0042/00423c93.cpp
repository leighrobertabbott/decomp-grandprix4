#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// imul + shrd by 11: low 32 bits of (value * scale) >> 11.
static inline uint32_t scaled11(uint32_t value, int32_t scale) {
    return static_cast<uint32_t>((static_cast<int64_t>(static_cast<int32_t>(value)) * scale) >> 11);
}

// Adds the scaled increment to a running value and keeps the result only while the add did not
// change the sign of an operand pair (a crude overflow guard): a zero sum, a sum whose sign
// differs from the old value, or a second increment that agrees with the sum leave the first sum
// in place; otherwise the second increment is added as well and any sign flip there gives 0.
static inline uint32_t advance(uint32_t state, uint32_t first, uint32_t second, int32_t scale) {
    const uint32_t sum = scaled11(first, scale) + state;
    if (sum == 0u) return 0u;
    if (((sum ^ state) & 0x80000000u) != 0u) return sum;
    if (((second ^ sum) & 0x80000000u) == 0u) return sum;
    const uint32_t next = scaled11(second, scale) + sum;
    if (((sum ^ next) & 0x80000000u) == 0u) return next;
    return 0u;
}

// (int32)(next + state) >> 1, times the scale, >> 16 (low dword): the amount added to the
// accumulator. `high` receives the upper dword of the 64-bit product (edx).
static inline uint32_t accumulate(uint32_t next, uint32_t state, int32_t scale, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(next + state) >> 1) * scale;
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(product >> 16);
}

// FUNCTION: GP4 0x00423c93
void __cdecl FUN_00423c93(gp4::Regs* r) {
    uint32_t high = 0;

    uint32_t state = GP4_GLOBAL(uint32_t, 0x00607f34);
    uint32_t next = advance(state,
        GP4_GLOBAL(uint32_t, 0x00607fc4) + GP4_GLOBAL(uint32_t, 0x00607ff4),
        GP4_GLOBAL(uint32_t, 0x00607fdc) + GP4_GLOBAL(uint32_t, 0x00607fe8),
        GP4_GLOBAL(int32_t, 0x00607fa0));
    GP4_GLOBAL(uint32_t, 0x00607f34) = next;
    GP4_GLOBAL(uint32_t, 0x00607f28) += accumulate(next, state, GP4_GLOBAL(int32_t, 0x00607fa0), &high);

    if (GP4_GLOBAL(uint8_t, 0x0060827f) == 0u) {
        state = GP4_GLOBAL(uint32_t, 0x00607f38);
        next = advance(state,
            GP4_GLOBAL(uint32_t, 0x00607fc8) + GP4_GLOBAL(uint32_t, 0x00607ff8),
            GP4_GLOBAL(uint32_t, 0x00607fe0) + GP4_GLOBAL(uint32_t, 0x00607fec),
            GP4_GLOBAL(int32_t, 0x00607fa0));
        GP4_GLOBAL(uint32_t, 0x00607f38) = next;
        GP4_GLOBAL(uint32_t, 0x00607f2c) += accumulate(next, state, GP4_GLOBAL(int32_t, 0x00607fa0), &high);
    }

    state = GP4_GLOBAL(uint32_t, 0x00607f3c);
    next = advance(state,
        GP4_GLOBAL(uint32_t, 0x00607fcc) + GP4_GLOBAL(uint32_t, 0x00607ffc),
        GP4_GLOBAL(uint32_t, 0x00607fe4) + GP4_GLOBAL(uint32_t, 0x00607ff0),
        GP4_GLOBAL(int32_t, 0x00607fa0));
    GP4_GLOBAL(uint32_t, 0x00607f3c) = next;
    const uint32_t delta = accumulate(next, state, GP4_GLOBAL(int32_t, 0x00607fa0), &high);
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00607f30);
    const uint32_t after = before + delta;
    GP4_GLOBAL(uint32_t, 0x00607f30) = after;

    // eax leaves holding the last increment, edx its product's upper dword, ebp the old third
    // state value; the flags are those of the final "add [0x607f30], eax".
    r->eax = delta;
    r->edx = high;
    r->ebp = state;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(before, delta, after);
}
GP4_IMPL(0x00423c93, FUN_00423c93, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
