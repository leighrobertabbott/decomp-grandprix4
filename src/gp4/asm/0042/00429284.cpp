#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline void takeCallee(gp4::Regs* r, const gp4::Regs& state) {
    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->edx = state.edx;
    r->esi = state.esi;
    r->edi = state.edi;
    r->ebp = state.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (state.eflags & 0x8d5u);
}

// Dispatch on the side of the ring split (edi against DAT_00891500) and the mode in ecx:
// below the split with ecx == 1 runs 0x00429162; at or above it with ecx != 1 runs 0x004291f4.
// Otherwise only the flags of `cmp ecx, 1` remain.
// FUNCTION: GP4 0x00429284
void __cdecl FUN_00429284(gp4::Regs* r) {
    const bool below = r->edi < GP4_GLOBAL(uint32_t, 0x00891500);
    const bool modeOne = r->ecx == 1u;

    if (below && modeOne) {
        gp4::Regs state = *r;
        state.st_in = 0;
        state.st_out = 0;
        gp4::call_regs(0x00429162, state);
        takeCallee(r, state);
        return;
    }
    if (!below && !modeOne) {
        gp4::Regs state = *r;
        state.st_in = 0;
        state.st_out = 0;
        gp4::call_regs(0x004291f4, state);
        takeCallee(r, state);
        return;
    }
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(r->ecx, 1u);
}
GP4_IMPL(0x00429284, FUN_00429284, "regs(eax:i32, ecx:u32[0..2], edx:i32, edi:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+pitw_00852c44+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0")
