#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `sub left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0042fa30
// Walks the dword table at 0x006077e4 from byte offset [0x006077dc]-4 down to 0 in steps of 4
// and zeroes every entry equal to ebx. Leaves eax as the first negative offset and the flags of
// the last `sub eax, 4`.
void __cdecl FUN_0042fa30(gp4::Regs* r) {
    uint8_t* table = GP4_ARRAY(uint8_t, 0x006077e4);
    const uint32_t target = r->ebx;
    uint32_t offset = GP4_GLOBAL(uint32_t, 0x006077dc);
    uint32_t flags = subFlags32(offset, 4u);
    offset -= 4u;
    while (static_cast<int32_t>(offset) >= 0) {
        if (GP4_FIELD(uint32_t, table, offset) == target) {
            GP4_FIELD(uint32_t, table, offset) = 0u;
        }
        flags = subFlags32(offset, 4u);
        offset -= 4u;
    }
    r->eax = offset;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042fa30, FUN_0042fa30, "regs(ebx:u32[0..2]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0042fa52")
