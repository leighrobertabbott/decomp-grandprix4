#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Finds the first of the 22 car records at 0x0066fef0 (stride 0x41c) whose byte +0x7a has bits 7 and 5 clear
// and byte +0x8d has bits 1 and 2 clear and for which 0x00443895 (esi = record) reports no carry. CF is
// cleared (clc, the other flags are the callee's) when such a car exists and set (stc, the other flags those of
// the final `add esi, 0x41c`) when none does. esi and ecx are saved and restored; every other register is
// untouched.
// FUNCTION: GP4 0x00443955
void __cdecl FUN_00443955(gp4::Regs* r) {
    uint32_t esi = 0x0066fef0u;
    uint32_t before = esi;
    for (int remaining = 0x16; remaining != 0; --remaining) {
        const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi));
        if ((record[0x7a] & 0x80u) == 0u && (record[0x7a] & 0x20u) == 0u
            && (record[0x8d] & 2u) == 0u && (record[0x8d] & 4u) == 0u) {
            gp4::Regs call = *r;
            call.ecx = static_cast<uint32_t>(remaining);
            call.esi = esi;
            gp4::call_regs(0x00443895, call);
            if ((call.eflags & 1u) == 0u) {
                r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d4u);
                return;
            }
        }
        before = esi;
        esi += 0x41cu;
    }
    r->eflags = (r->eflags & ~0x8d5u) | (addFlags32(before, 0x41cu) | 1u);
}
GP4_IMPL(0x00443955, FUN_00443955, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+cars_0066fef0+cars_00433d8d+flags_00432dcf+state_00432f18+state_004320f2+ring_008943a0")
