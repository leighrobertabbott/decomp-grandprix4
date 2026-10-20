#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return byteParity(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a 32-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return byteParity(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// When the gate byte 0x00852d8c is zero and the car record pointer [0x0066d820] is non-null with bit 7 of its byte
// +0x78 set, raises bit 0 of the byte 0x008914e0. esi is loaded with the pointer once the gate passes; the flags are
// those of the last test / or on the path taken.
// FUNCTION: GP4 0x00445483
void __cdecl FUN_00445483(gp4::Regs* r) {
    const uint32_t gate = GP4_GLOBAL(uint8_t, 0x00852d8c);
    uint32_t flags = logicFlags8(gate);
    if (gate == 0u) {
        const uint32_t record = GP4_GLOBAL(uint32_t, 0x0066d820);
        r->esi = record;
        flags = logicFlags32(record);
        if (record != 0u) {
            const uint32_t state = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(record))[0x78] & 0x80u;
            flags = logicFlags8(state);
            if (state != 0u) {
                const uint8_t raised = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x008914e0) | 1u);
                GP4_GLOBAL(uint8_t, 0x008914e0) = raised;
                flags = logicFlags8(raised);
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00445483, FUN_00445483, "regs() -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01")
