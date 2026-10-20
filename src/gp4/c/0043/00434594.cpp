#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00434594
void __cdecl FUN_00434594(gp4::Regs* r) {
    // clear the nine dwords at the table pointed to by DAT_0062ff64
    const uint32_t start = GP4_GLOBAL(uint32_t, 0x0062ff64);
    uint32_t* dst = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(start));
    for (uint32_t i = 0; i < 9u; ++i) {
        dst[i] = 0u;
    }
    // edi ends one past the table, ecx counts down to zero; the flags are those of the
    // last `add edi, 4`
    const uint32_t lhs = start + 32u;
    const uint32_t result = lhs + 4u;
    const uint32_t flags = (result < lhs ? 1u : 0u)
        | parityFlag(result)
        | (((lhs ^ 4u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(lhs ^ 4u) & (lhs ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->edi = result;
    r->ecx = 0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00434594, FUN_00434594, "regs() -> (ecx, edi, cf, pf, af, zf, sf, of); globals=layout=state_00434583")
