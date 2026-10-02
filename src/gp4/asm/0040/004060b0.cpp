#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) left by `cmp left, right`.
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t flags = parityFlag(result);
    if (left < right) flags |= 0x001u;
    if (((left ^ right ^ result) & 0x10u) != 0u) flags |= 0x010u;
    if (result == 0u) flags |= 0x040u;
    if ((result & 0x80000000u) != 0u) flags |= 0x080u;
    if ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u) flags |= 0x800u;
    return flags;
}

// FUNCTION: GP4 0x004060b0
void __cdecl FUN_004060b0(gp4::Regs* r) {
    const uint32_t entry = r->edi;
    const uint32_t boundary = GP4_GLOBAL(uint32_t, 0x00891500);
    uint32_t result = entry;
    uint32_t flags;

    if (entry < boundary) {
        const uint32_t first = GP4_GLOBAL(uint32_t, 0x00866ecc);
        if (entry == first) {
            flags = cmpFlags(entry, first);
            result = 0x008943a0u;
        } else {
            const uint32_t second = GP4_GLOBAL(uint32_t, 0x00866f00);
            flags = cmpFlags(entry, second);
            if (entry == second) {
                result = GP4_GLOBAL(uint32_t, 0x00891500);
            }
        }
    } else {
        const uint32_t third = GP4_GLOBAL(uint32_t, 0x00866ef8);
        flags = cmpFlags(entry, third);
        if (entry == third) {
            result = GP4_GLOBAL(uint32_t, 0x00866f04);
        }
    }

    r->edi = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004060b0, FUN_004060b0, "regs(edi:u32) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
