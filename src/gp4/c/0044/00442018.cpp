#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Picks the leading car record into esi (0x0043b7eb); when bit 1 of its byte +0x8d is clear, runs 0x0044202c.
// Otherwise only the flags of the `test` remain. Registers and flags flow through both callees.
// FUNCTION: GP4 0x00442018
void __cdecl FUN_00442018(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0043b7eb, call);

    const uint8_t flags8d = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(call.esi) + 0x8du);
    if ((flags8d & 2u) == 0u) {
        call.eflags = (call.eflags & ~0x8d5u) | testFlags8(flags8d & 2u);
        gp4::call_regs(0x0044202c, call);
    } else {
        call.eflags = (call.eflags & ~0x8d5u) | testFlags8(flags8d & 2u);
    }
    *r = call;
}
GP4_IMPL(0x00442018, FUN_00442018, "regs() -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+lap_0043b482")
