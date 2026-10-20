#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of `cmp left, right` on an operand of the width given by mask / sign bit.
static inline uint32_t compareFlags(uint32_t left, uint32_t right, uint32_t mask, uint32_t sign) {
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & sign) != 0u ? 0x800u : 0u);
}

// Flags of `test` on a result of the given sign bit: CF = OF = AF = 0.
static inline uint32_t testFlags(uint32_t result, uint32_t sign) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u);
}

// The decision chain behind the routine's carry result; `flags` receives the flags of the last
// flag-writing instruction executed on the way (popal restores nothing but the registers).
// Returns true when the original ends with stc.
static bool probe(gp4::Regs* r, uint32_t* flags) {
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t selfAddress = r->esi;

    const uint32_t gate = GP4_GLOBAL(uint8_t, 0x006259dc);
    *flags = compareFlags(gate, 0xffu, 0xffu, 0x80u);
    if (gate == 0xffu) return true;

    const uint32_t status = GP4_FIELD(uint8_t, self, 0x78) & 8u;
    *flags = testFlags(status, 0x80u);
    if (status != 0u) return true;

    const uint32_t busy = GP4_FIELD(uint32_t, self, 0x44);
    *flags = testFlags(busy, 0x80000000u);
    if (busy != 0u) return false;

    const uint32_t position = GP4_FIELD(uint32_t, self, 0x1c);
    *flags = compareFlags(position, GP4_GLOBAL(uint32_t, 0x00891500), 0xffffffffu, 0x80000000u);
    if (position >= GP4_GLOBAL(uint32_t, 0x00891500)) return true;

    *flags = compareFlags(selfAddress, GP4_GLOBAL(uint32_t, 0x0066d820), 0xffffffffu, 0x80000000u);
    if (selfAddress == GP4_GLOBAL(uint32_t, 0x0066d820)) return false;

    const uint32_t focus = GP4_GLOBAL(uint8_t, 0x007c2e6c);
    *flags = compareFlags(focus, 0u, 0xffu, 0x80u);
    if (focus != 0u) {
        *flags = compareFlags(selfAddress, GP4_GLOBAL(uint32_t, 0x0066d82c), 0xffffffffu, 0x80000000u);
        if (selfAddress == GP4_GLOBAL(uint32_t, 0x0066d82c)) return false;
    }

    const uint32_t player = GP4_GLOBAL(uint8_t, 0x007aafd4);
    *flags = testFlags(player, 0x80u);
    if (player != 0u) {
        *flags = compareFlags(selfAddress, GP4_GLOBAL(uint32_t, 0x0066d828), 0xffffffffu, 0x80000000u);
        if (selfAddress == GP4_GLOBAL(uint32_t, 0x0066d828)) return false;
    }

    const uint32_t marker = GP4_FIELD(uint8_t, self, 0x203) & 4u;
    *flags = testFlags(marker, 0x80u);
    if (marker != 0u) return false;

    gp4::Regs callee = *r;
    callee.edi = position;
    gp4::call_regs(0x00437888, callee);
    *flags = callee.eflags;
    return (callee.eflags & 1u) != 0u;
}

// pushal ... popal: every GPR is restored; the flags of the last flag writer stay, with CF
// replaced by the final clc (false) / stc (true).
// FUNCTION: GP4 0x004378fc
void __cdecl FUN_004378fc(gp4::Regs* r) {
    uint32_t flags = 0;
    const bool carry = probe(r, &flags);
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | (carry ? 1u : 0u);
}
GP4_IMPL(0x004378fc, FUN_004378fc, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=layout=state_00437888")
