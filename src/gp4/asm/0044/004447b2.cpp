#include <gp4/gp4.h>

// Connected-component flood fill over the 23-part adjacency masks at 0x0062a15c.
// Input: ebp = bit mask of candidate parts (bits 0..22), eax = value returned untouched when no
// bit is set. Output: eax = the group of parts reachable from the lowest set bit of ebp (CF clear),
// or CF set with eax unchanged when ebp has no bit in 0..22. Every other register is preserved;
// the flags always end as ZF|PF set, AF/SF/OF clear (the last flag writer produced zero).
// FUNCTION: GP4 0x004447b2
void __cdecl FUN_004447b2(gp4::Regs* r) {
    uint32_t pending = r->ebp;
    uint32_t group = r->eax;
    uint32_t carry = 1u;

    uint32_t bit = 1u;
    for (int part = 0; part != 0x17; part++, bit <<= 1) {
        if ((bit & pending) == 0u) {
            continue;
        }
        group = GP4_ARRAY(uint32_t, 0x0062a15c)[part] & pending;
        pending &= ~bit;
        for (int pass = 0x17; pass != 0; pass--) {
            bool merged = false;
            uint32_t other = 1u;
            for (int j = 0; j != 0x17; j++, other <<= 1) {
                if ((other & pending) != 0u && (GP4_ARRAY(uint32_t, 0x0062a15c)[j] & group) != 0u) {
                    group |= GP4_ARRAY(uint32_t, 0x0062a15c)[j] & pending;
                    pending &= ~other;
                    merged = true;
                }
            }
            if (!merged) {
                break;
            }
        }
        carry = 0u;
        break;
    }

    r->eax = group;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u | carry;
}
GP4_IMPL(0x004447b2, FUN_004447b2, "regs(eax:u32, ebp:u32) -> (eax, cf, pf, af, zf, sf, of)")
