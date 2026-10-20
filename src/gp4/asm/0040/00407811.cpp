#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by `or eax, eax`: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t orFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// The three dispatch targets are kept in separate helpers so each call address survives code folding.
static __declspec(noinline) void runZero(gp4::Regs& call) {
    gp4::call_regs(0x0040759e, call);
}

static __declspec(noinline) void runNegative(gp4::Regs& call) {
    gp4::call_regs(0x0040729a, call);
}

static __declspec(noinline) void runPositive(gp4::Regs& call) {
    gp4::call_regs(0x004075d0, call);
}

// FUNCTION: GP4 0x00407811
// Dispatch on eax with ebx/ecx/edx/ebp/esi/edi saved and restored: zero runs 0x0040759e, a negative value
// 0x0040729a, a positive one 0x004075d0 (all on the register file the caller passed in). eax is the callee's
// result; the flags are those the callee left.
void __cdecl FUN_00407811(gp4::Regs* r) {
    gp4::Regs saved = *r;
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eflags = (call.eflags & ~0x8d5u) | orFlags32(call.eax);

    if (call.eax == 0u) {
        runZero(call);
    } else if (static_cast<int32_t>(call.eax) < 0) {
        runNegative(call);
    } else {
        runPositive(call);
    }

    saved.eax = call.eax;
    saved.eflags = (saved.eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
    *r = saved;
}
GP4_IMPL(0x00407811, FUN_00407811, "regs(eax:i32, edi:rec[0x8943a0,0x160,0..11]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004381ab+ring_00406ac7+trackgeo_008943a0+state_0040705f+frac_0040705f+state_00407af6+state_0040759e+trackfar_008943a0")
