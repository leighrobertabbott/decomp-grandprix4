#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit TEST (CF = OF = AF = 0) with result `value`.
static inline uint32_t testFlags8(uint32_t value) {
    value &= 0xffu;
    return byteParity(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 16-bit AND (CF = OF = AF = 0) with result `value`.
static inline uint32_t logicFlags16(uint32_t value) {
    value &= 0xffffu;
    return byteParity(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit TEST (CF = OF = AF = 0) with result `value`.
static inline uint32_t testFlags32(uint32_t value) {
    return byteParity(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Returns the arithmetic flags the original leaves behind. ecx and esi are saved/restored by the original.
static uint32_t scanCars(const gp4::Regs* r) {
    const uint8_t gate = GP4_GLOBAL(uint8_t, 0x0066d87a);
    if ((gate & 0x80u) != 0u) {
        return testFlags8(gate);
    }

    const uint16_t phase = static_cast<uint16_t>((GP4_GLOBAL(uint16_t, 0x0066d878) + 1u) & 0xfu);
    GP4_GLOBAL(uint16_t, 0x0066d878) = phase;
    uint32_t flags = logicFlags16(phase);
    if (phase != 0u) {
        return flags;
    }

    uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0);
    for (uint32_t remaining = 0x16; remaining != 0u; --remaining) {
        const uint32_t retired = car[0x7a] & 0x20u;
        flags = testFlags8(retired);
        if (retired == 0u) {
            const uint32_t flagged = car[0x8d] & 4u;
            flags = testFlags8(flagged);
            if (flagged == 0u) {
                const uint32_t enabled = GP4_GLOBAL(uint32_t, 0x0066d824);
                flags = testFlags32(enabled);
                if (enabled == 0u) {
                    return flags;
                }
                gp4::Regs callee = *r;
                callee.st_in = 0;
                callee.st_out = 0;
                callee.ecx = remaining;
                callee.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
                gp4::call_regs(0x00443895, callee);
                flags = callee.eflags & 0x8d5u;
                if ((flags & 1u) == 0u) {
                    return flags;
                }
            }
        }
        flags = addFlags32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car)), 0x41cu);
        car += 0x41c;
    }
    GP4_GLOBAL(uint8_t, 0x0066d87c) = 0x80;
    return flags;
}

// Every 16th call (the low nibble of the word counter [0x0066d878] wraps to zero; skipped while the byte
// [0x0066d87a] is negative) walks the 22 car records at 0x0066fef0 (stride 0x41c): for each car not flagged
// by +0x7a bit 5 / +0x8d bit 2 it asks 0x00443895 (CF = 1: keep scanning) unless [0x0066d824] is zero, and
// stops at the first car that answers CF = 0. If all of them pass, [0x0066d87c] = 0x80. ecx and esi are
// saved/restored; the flags are those of the last flag writer.
// FUNCTION: GP4 0x004438f8
void __cdecl FUN_004438f8(gp4::Regs* r) {
    r->eflags = (r->eflags & ~0x8d5u) | scanCars(r);
}
GP4_IMPL(0x004438f8, FUN_004438f8, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040e949+cars_0066fef0+flags_00432dcf+cars_00433d8d+state_00432f18+state_004320f2")
