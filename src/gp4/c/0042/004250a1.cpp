#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `add left, right` (CF PF AF ZF SF OF).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Runs the 0x00424eea per-car update over the 22 records (stride 0x41c) at 0x0066fef0:
// `mov ecx, 0x16; mov esi, 0x66fef0; do { call; add esi, 0x41c } while (--ecx)`. The registers the
// callee changes carry from one iteration to the next; the flags are those of the final add.
// FUNCTION: GP4 0x004250a1
void __cdecl FUN_004250a1(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.ecx = 0x16u;
    state.esi = 0x0066fef0u;

    uint32_t flags;
    do {
        gp4::call_regs(0x00424eea, state);
        flags = addFlags32(state.esi, 0x41cu);
        state.esi += 0x41cu;
        state.ecx -= 1u;
    } while (state.ecx != 0u);

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->edx = state.edx;
    r->esi = state.esi;
    r->edi = state.edi;
    r->ebp = state.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004250a1, FUN_004250a1, "regs() -> (eax, ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
