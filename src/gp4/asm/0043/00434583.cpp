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

// Sets bit `edx` of the bit-mask row `eax` (dwords) in the table at pointer global 0x0062ff64:
// `bts dword [ptr + eax*4], edx`. eax and edx are saved and restored. The flags are those of the
// address `add eax, [0x62ff64]`, with CF replaced by the bit's old value (bts defines only CF;
// the oracle model leaves the other flags as the add made them).
// FUNCTION: GP4 0x00434583
void __cdecl FUN_00434583(gp4::Regs* r) {
    const uint32_t scaled = r->eax << 2;
    const uint32_t table = GP4_GLOBAL(uint32_t, 0x0062ff64);
    const uint32_t bitOffset = r->edx;

    // register bit offset on a memory operand: signed dword displacement plus bit within it
    uint32_t* word = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(scaled + table))
        + (static_cast<int32_t>(bitOffset) >> 5);
    const uint32_t mask = 1u << (bitOffset & 31u);
    const uint32_t old = *word;
    *word = old | mask;

    const uint32_t addFlags = addFlags32(scaled, table);
    r->eflags = (r->eflags & ~0x8d5u) | (addFlags & ~1u) | ((old & mask) != 0u ? 1u : 0u);
}
GP4_IMPL(0x00434583, FUN_00434583, "regs(eax:u32[0..15], edx:u32[0..31]) -> (cf, pf, af, zf, sf, of); globals=layout=state_00434583")
