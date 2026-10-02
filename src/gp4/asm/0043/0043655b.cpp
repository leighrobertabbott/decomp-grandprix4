#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0043655b
void __cdecl FUN_0043655b(gp4::Regs* r) {
    const uint32_t left = r->esi;
    const uint32_t right = GP4_GLOBAL(uint32_t, 0x00679d30);
    uint32_t flags;
    if (left != right) {
        // cmp esi, [0x679d30]
        const uint32_t result = left - right;
        flags = (left < right ? 1u : 0u)
            | parityFlag(result)
            | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | (result == 0u ? 0x40u : 0u)
            | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
            | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    } else {
        const uint32_t ax = r->eax & 0xffffu;
        const uint32_t word = GP4_GLOBAL(uint16_t, 0x0062593e);
        if (ax != word) {
            // cmp ax, [0x62593e]
            const uint32_t result = (ax - word) & 0xffffu;
            flags = (ax < word ? 1u : 0u)
                | parityFlag(result)
                | (((ax ^ word ^ result) & 0x10u) != 0u ? 0x10u : 0u)
                | (result == 0u ? 0x40u : 0u)
                | ((result & 0x8000u) != 0u ? 0x80u : 0u)
                | (((ax ^ word) & (ax ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
        } else {
            // or ax, ax: CF=OF=0, AF cleared in the pinned model
            flags = parityFlag(ax)
                | (ax == 0u ? 0x40u : 0u)
                | ((ax & 0x8000u) != 0u ? 0x80u : 0u);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043655b, FUN_0043655b, "regs(eax:u32, esi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
