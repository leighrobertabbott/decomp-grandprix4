#include <gp4/gp4.h>

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004159a4
void __cdecl FUN_004159a4(gp4::Regs* r) {
    gp4::Regs callee = *r;
    gp4::call_regs(0x00401462, callee);

    const int64_t first = static_cast<int64_t>(static_cast<int32_t>(callee.eax & 0xffu))
        * GP4_GLOBAL(int32_t, 0x00603a28);
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 8)
        + GP4_GLOBAL(uint32_t, 0x00603a2c);

    int32_t factor = GP4_GLOBAL(int32_t, 0x0063b834) - GP4_GLOBAL(int32_t, 0x0066d814);
    if (factor < 0) {
        factor = 0;
    }

    const int64_t second = static_cast<int64_t>(static_cast<int32_t>(scaled)) * factor;
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14);
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x00603a30);

    r->eax = shifted + offset;
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(shifted, offset);
}
GP4_IMPL(0x004159a4, FUN_004159a4, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
