#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of cmp / sub left, right (32-bit)
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Three passes ("loop" on ecx = 3): the two per-record updates 0x0043da76 and 0x0043d76b run with
// esi and the 0x160-byte ring record edi, then edi advances to the next ring record, wrapping at
// the ring bounds exactly like the forward walk of 0x0043dd13. The flags are those of the last
// compare or subtraction (the loop instruction does not touch them).
// FUNCTION: GP4 0x0043e059
void __cdecl FUN_0043e059(gp4::Regs* r) {
    uint32_t flags = 0u;
    r->ecx = 3u;
    do {
        gp4::call_regs(0x0043da76, *r);
        gp4::call_regs(0x0043d76b, *r);

        uint32_t next = r->edi + 0x160u;
        if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0u) {
            const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00866ef8);
            flags = subFlags(next, limit);
            if (next >= limit) {
                flags = subFlags(next, 0x160u);
                next -= 0x160u;
            }
        } else {
            const uint32_t low = GP4_GLOBAL(uint32_t, 0x00866ecc);
            flags = subFlags(next, low);
            if (next >= low) {
                const uint32_t high = GP4_GLOBAL(uint32_t, 0x00891500);
                flags = subFlags(next, high);
                if (next < high) {
                    const uint32_t shift = GP4_GLOBAL(uint32_t, 0x00866ec4);
                    flags = subFlags(next, shift);
                    next -= shift;
                } else {
                    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00866ef8);
                    flags = subFlags(next, limit);
                    if (next >= limit) {
                        flags = subFlags(limit, 0x160u);
                        next = limit - 0x160u;
                    }
                }
            }
        }
        r->edi = next;
        r->ecx = r->ecx - 1u;
    } while (r->ecx != 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043e059, FUN_0043e059, "regs(eax:u32, esi:ptr[0xf6]:bytes, edi:rec[0x8943a0,0x160,0..29]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0")
