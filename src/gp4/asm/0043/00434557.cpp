#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// Tests bit `esi` (car 0..21) of the bit-mask row `eax` (dwords of the table 0x00433fe0 returns).
// CF = 1 when the bit is set. In that case, when eax == 4 it is replaced by the dword at the start
// of car esi's 0x1c-byte record at 0x0062ffac. The flags are those of the last flag writer (the
// row address add, or cmp eax, 4) with CF forced by clc / stc; ecx is saved and restored.
// FUNCTION: GP4 0x00434557
void __cdecl FUN_00434557(gp4::Regs* r) {
    const uint32_t row = r->eax;
    const uint32_t car = r->esi;

    const uint32_t scaled = row << 2;
    const uint32_t table = gp4::cdecl_<0x00433fe0, uint32_t>();

    // bt dword [scaled + table], car: a register bit offset addresses dword (car >> 5)
    const uint32_t* word = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(scaled + table))
        + (static_cast<int32_t>(car) >> 5);
    const bool set = ((*word >> (car & 31u)) & 1u) != 0u;

    uint32_t eax = row;
    uint32_t flags;
    if (set) {
        const uint32_t* record = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(0x0062ffacu + car * 0x1cu));
        flags = subFlags32(row, 4u) | 1u;
        if (row == 4u) {
            eax = *record;
        }
    } else {
        flags = addFlags32(scaled, table) & ~1u;
    }

    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00434557, FUN_00434557, "regs(eax:u32[0..15], esi:u32[0..21]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_00434557")
