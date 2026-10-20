#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// EFLAGS arithmetic bits of the byte operation left - right (sub / cmp)
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// FUNCTION: GP4 0x0043e812
void __cdecl FUN_0043e812(gp4::Regs* r) {
    uint32_t rec = r->edi;
    uint8_t level = (uint8_t)r->eax;
    uint8_t diff;
    uint32_t flags;

    // Walk backwards through the 0x160-byte record ring, raising each record's
    // byte +0x6b to level + 1 while the record is more than one step above it.
    for (;;) {
        if (rec >= GP4_GLOBAL(uint32_t, 0x00891500)) {
            rec -= 0x160u;
            if (rec < GP4_GLOBAL(uint32_t, 0x00891500)) {
                rec = GP4_GLOBAL(uint32_t, 0x00891500);
            }
        } else {
            rec -= 0x160u;
            if (rec < GP4_GLOBAL(uint32_t, 0x00891504)) {
                rec += GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        }
        const uint8_t cur = *(uint8_t*)(rec + 0x6bu);
        diff = (uint8_t)(cur - level);
        flags = subFlags8(cur, level);
        if ((diff & 0x80u) != 0u) {
            break;
        }
        flags = subFlags8(diff, 1u);
        if ((int8_t)diff <= 1) {
            break;
        }
        level = (uint8_t)(level + 1u);
        *(uint8_t*)(rec + 0x6bu) = level;
    }

    r->eax = (r->eax & ~0xffu) | level;
    r->edx = (r->edx & ~0xffu) | diff;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043e812, FUN_0043e812, "regs(eax:u32, edx:u32, edi:u32[0x8944e0..0x895000]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0040705f")
