#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by `cmp byte, 0`: CF = OF = AF = 0, PF/ZF/SF from the byte.
static inline uint32_t cmpZeroFlags8(uint32_t value) {
    return parityFlag8(value)
        | ((value & 0xffu) == 0u ? 0x40u : 0u)
        | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x004020ad
// pushal ... popal wrapper. Mode byte 0x63963d: with zero, 0x00401fb4 first builds the rotation from the
// dwords at 0x00638dd0..d8; then, again by the mode byte, either 0x00402051 (non-zero) or 0x00402081 followed
// by 0x00402038 (zero) run. Every register is restored by popal; the flags are those the last callee left,
// which keep the flags of the `cmp byte [0x63963d], 0` the callees were entered with.
void __cdecl FUN_004020ad(gp4::Regs* r) {
    gp4::Regs saved = *r;
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    uint8_t mode = GP4_GLOBAL(uint8_t, 0x0063963d);
    call.eflags = (call.eflags & ~0x8d5u) | cmpZeroFlags8(mode);
    if (mode == 0u) {
        gp4::call_regs(0x00401fb4, call);
    }

    mode = GP4_GLOBAL(uint8_t, 0x0063963d);
    call.eflags = (call.eflags & ~0x8d5u) | cmpZeroFlags8(mode);
    if (mode == 0u) {
        gp4::call_regs(0x00402081, call);
        gp4::call_regs(0x00402038, call);
    } else {
        gp4::call_regs(0x00402051, call);
    }

    saved.eflags = (saved.eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
    *r = saved;
}
GP4_IMPL(0x004020ad, FUN_004020ad, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860")
