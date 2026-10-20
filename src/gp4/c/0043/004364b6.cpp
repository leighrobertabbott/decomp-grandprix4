#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a logic result (test / and): CF = OF = AF = 0. `mask` is the operand width mask and
// `sign` its sign bit.
static inline uint32_t logicFlags(uint32_t result, uint32_t mask, uint32_t sign) {
    return parityFlag(result) | ((result & mask) == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x004364b6
// With the word at 0x0066d898 zero and the due time 0x0066d83c either zero or already passed (the
// clock 0x0066d814 minus it is negative), the car in esi gets the clock at +0xf0. When the byte
// 0x007aafd4 is set, byte 0x007a90fc is cleared, bit 7 of byte +0x78 is cleared and 0x00444d97
// queues the car's +0xf0 / +0xf2 words. Registers are preserved; the flags are those of the last
// flag writer on the path taken.
void __cdecl FUN_004364b6(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t word = GP4_GLOBAL(uint16_t, 0x0066d898);
    uint32_t flags = logicFlags(word, 0xffffu, 0x8000u);
    if (word == 0u) {
        bool proceed = true;
        const uint32_t due = GP4_GLOBAL(uint32_t, 0x0066d83c);
        if (due != 0u) {
            const uint32_t left = 0u - due;
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            flags = addFlags(left, clock);
            proceed = ((left + clock) & 0x80000000u) != 0u;
        }
        if (proceed) {
            GP4_FIELD(uint32_t, car, 0xf0) = GP4_GLOBAL(uint32_t, 0x0066d814);
            const uint32_t enabled = GP4_GLOBAL(uint8_t, 0x007aafd4);
            flags = logicFlags(enabled, 0xffu, 0x80u);
            if (enabled != 0u) {
                GP4_GLOBAL(uint8_t, 0x007a90fc) = 0;
                const uint32_t cleared = GP4_FIELD(uint8_t, car, 0x78) & 0x7fu;
                GP4_FIELD(uint8_t, car, 0x78) = static_cast<uint8_t>(cleared);
                flags = logicFlags(cleared, 0xffu, 0x80u);

                gp4::Regs call = *r;
                call.st_in = 0;
                call.st_out = 0;
                gp4::call_regs(0x00444d97, call);
                flags = call.eflags & 0x8d5u;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004364b6, FUN_004364b6, "regs(esi:ptr[0x100]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=clock_0066d814+state_0043b920")
