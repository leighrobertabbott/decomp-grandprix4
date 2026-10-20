#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of the 32-bit "cmp lhs, rhs"
static inline uint32_t cmpFlags32(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = lhs - rhs;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// When DAT_0062a96c is not -1, runs the group reset 0x00431f6e with eax = DAT_0062a96c (the group
// index) and edx = DAT_0062a968 (the record base), then marks it done with -1. With -1 the flags
// are those of the compare (equal).
// FUNCTION: GP4 0x00431fd7
void __cdecl FUN_00431fd7(gp4::Regs* r) {
    const uint32_t group = GP4_GLOBAL(uint32_t, 0x0062a96c);
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(group, 0xffffffffu);
    if (group == 0xffffffffu) {
        return;
    }
    r->edx = GP4_GLOBAL(uint32_t, 0x0062a968);
    r->eax = GP4_GLOBAL(uint32_t, 0x0062a96c);
    gp4::call_regs(0x00431f6e, *r);
    GP4_GLOBAL(uint32_t, 0x0062a96c) = 0xffffffffu;
}
GP4_IMPL(0x00431fd7, FUN_00431fd7, "regs() -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_00431fd7")
