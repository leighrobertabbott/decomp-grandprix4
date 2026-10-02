#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00431002
void __cdecl FUN_00431002(gp4::Regs* r) {
    const uint32_t left = r->esi;
    const uint32_t right = GP4_GLOBAL(uint32_t, 0x00679d30);
    uint32_t flags;
    if (left != right) {
        // cmp esi, [0x679d30] leaves its flags
        const uint32_t result = left - right;
        flags = parityFlag(result)
            | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
            | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
            | (left < right ? 1u : 0u);
    } else {
        // equal: or eax, eax overwrites the flags
        const uint32_t value = r->eax;
        flags = parityFlag(value)
            | (value == 0u ? 0x40u : 0u)
            | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00431002, FUN_00431002, "regs(esi:u32, eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
