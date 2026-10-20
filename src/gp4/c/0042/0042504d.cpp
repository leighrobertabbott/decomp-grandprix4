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

// Walks the 22 car records (stride 0x41c) at 0x0066fef0; every record whose byte at +0x25e is
// 0xff goes through 0x00424cf5. The loop ends with ecx = 0 and esi one record past the array; the
// flags are those of the last `add esi, 0x41c` (loop leaves them alone).
// FUNCTION: GP4 0x0042504d
void __cdecl FUN_0042504d(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.ecx = 0x16u;
    state.esi = 0x0066fef0u;

    uint32_t flags;
    do {
        const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(state.esi));
        if (record[0x25e] == 0xffu) {
            gp4::call_regs(0x00424cf5, state);
        }
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
GP4_IMPL(0x0042504d, FUN_0042504d, "regs() -> (ecx, esi, cf, pf, af, zf, sf, of); wall=1200; globals=layout=cars_0042504d+state_0042458b+state_00420e1d+blk_00424553+state_0066d8c0+state_0041bce9+state_00444517+state_00424a21+state_0043c4d9")
