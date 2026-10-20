#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t commonFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return parityFlag(result) | ((lhs ^ rhs ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u);
}

// Pushes the pair (ebx, edx) onto two parallel dword stacks (values at 0x006077e4, keys at
// 0x00607860) whose byte offset is DAT_006077dc; the offset advances by 4 and the stack holds
// at most 0x74 / 4 entries. eax is preserved. The flags are those of "cmp eax, 0x74" when full,
// else of "add eax, 4".
// FUNCTION: GP4 0x0042a0fb
void __cdecl FUN_0042a0fb(gp4::Regs* r) {
    const uint32_t value = r->ebx;
    const uint32_t key = r->edx;
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x006077dc);

    uint32_t flags;
    if (offset < 0x74u) {
        *reinterpret_cast<uint32_t*>(GP4_ARRAY(uint8_t, 0x006077e4) + offset) = value;
        *reinterpret_cast<uint32_t*>(GP4_ARRAY(uint8_t, 0x00607860) + offset) = key;
        const uint32_t advanced = offset + 4u;
        GP4_GLOBAL(uint32_t, 0x006077dc) = advanced;
        // add eax, 4
        flags = commonFlags(offset, 4u, advanced)
            | (advanced < offset ? 1u : 0u)
            | ((~(offset ^ 4u) & (offset ^ advanced) & 0x80000000u) >> 20);
    } else {
        // cmp eax, 0x74
        const uint32_t difference = offset - 0x74u;
        flags = commonFlags(offset, 0x74u, difference)
            | (offset < 0x74u ? 1u : 0u)
            | (((offset ^ 0x74u) & (offset ^ difference) & 0x80000000u) >> 20);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042a0fb, FUN_0042a0fb, "regs(ebx:u32, edx:u32) -> (cf, pf, af, zf, sf, of); globals=layout=state_0042fa52")
