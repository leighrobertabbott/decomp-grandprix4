#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00437859
// ebx = car pointer [0x629e00], esi = car pointer [0x629e04]. For each of the two cars whose dword +0x40
// differs from +0x44, runs 0x00418cdd with eax = that car and edx = the other one. The flags are those of
// the last compare or of the last callee.
void __cdecl FUN_00437859(gp4::Regs* r) {
    r->ebx = GP4_GLOBAL(uint32_t, 0x00629e00);
    r->esi = GP4_GLOBAL(uint32_t, 0x00629e04);

    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    r->eax = GP4_FIELD(uint32_t, first, 0x40);
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(r->eax, GP4_FIELD(uint32_t, first, 0x44));
    if (r->eax != GP4_FIELD(uint32_t, first, 0x44)) {
        r->eax = r->ebx;
        r->edx = r->esi;
        gp4::call_regs(0x00418cdd, *r);
    }

    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = GP4_FIELD(uint32_t, second, 0x40);
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(r->eax, GP4_FIELD(uint32_t, second, 0x44));
    if (r->eax != GP4_FIELD(uint32_t, second, 0x44)) {
        r->eax = r->esi;
        r->edx = r->ebx;
        gp4::call_regs(0x00418cdd, *r);
    }
}
GP4_IMPL(0x00437859, FUN_00437859, "regs() -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=state_00437888+sel_004377a4+state_004371f3+state_00418cdd+pair_00437859")
