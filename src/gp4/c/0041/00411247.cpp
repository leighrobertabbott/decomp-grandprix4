#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t low = value & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    return (low & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `cmp a, b` on 8-bit operands.
static inline uint32_t compareFlags8(uint32_t a, uint32_t b) {
    a &= 0xffu;
    b &= 0xffu;
    const uint32_t diff = (a - b) & 0xffu;
    uint32_t flags = parityFlag8(diff);
    if (a < b) flags |= 0x01u;
    if (((a ^ b ^ diff) & 0x10u) != 0u) flags |= 0x10u;
    if (diff == 0u) flags |= 0x40u;
    if ((diff & 0x80u) != 0u) flags |= 0x80u;
    if ((((a ^ b) & (a ^ diff)) & 0x80u) != 0u) flags |= 0x800u;
    return flags;
}

// Flags of a logic result (and / test / or): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result, uint32_t signBit) {
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & signBit) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x00411247
// Teammate-proximity event for the car record in esi. Runs only while [0x007c2ea9] is 0x80, the
// frame counter [0x0063b830] is a multiple of 32 and bit 0 of byte +0xcd is set; then 0x0043c784
// (CF = 1: "close", nothing more) decides; otherwise, unless [0x007c2e75] is set, it queues the
// 0x00418fd0 event (ecx = 0xa, eax = edx = 0x1f4) and clears bit 0 of byte +0xcd. eax is left holding
// the masked frame counter (zero once it passes the gate); ecx and edx come back.
void __cdecl FUN_00411247(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    if (mode != 0x80u) {
        r->eflags = (r->eflags & ~0x8d5u) | compareFlags8(mode, 0x80u);
        return;
    }

    const uint32_t tick = GP4_GLOBAL(uint32_t, 0x0063b830) & 0x1fu;
    r->eax = tick;
    if (tick != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags(tick, 0x80000000u);
        return;
    }

    if ((GP4_FIELD(uint8_t, car, 0xcd) & 1u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    gp4::call_regs(0x0043c784, state);
    if ((state.eflags & 1u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | (state.eflags & 0x8d5u);
        return;
    }

    if (GP4_GLOBAL(uint8_t, 0x007c2e75) == 0u) {
        gp4::Regs event = state;
        event.ecx = 0xau;
        event.eax = 0x1f4u;
        event.edx = 0x1f4u;
        gp4::call_regs(0x00418fd0, event);
    }
    const uint8_t cleared = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0xcd) & 0xfeu);
    GP4_FIELD(uint8_t, car, 0xcd) = cleared;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags(cleared, 0x80u);
}
GP4_IMPL(0x00411247, FUN_00411247, "regs(eax:u32, esi:ptr[0x2c4]:layout=car_0043c677) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00411109+cars_0066fef0+state_00411017+lap_0043b482+state_0041104f+state_00432330+state_0043c677+segw_0043c608+state_0066d8c0")
