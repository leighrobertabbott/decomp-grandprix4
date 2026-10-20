#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sort-key probe: looks up the key of list index 0 in the table chosen by the selector
// [0x0089151c] (0x00516daa) and reports through the carry flag whether it equals 0x20000000:
// `cmp eax, 0x20000000` / `stc` when equal, else `clc`. eax = the key, edx = the selector; the other
// flags are those of the compare.
// FUNCTION: GP4 0x00516df4
void __cdecl FUN_00516df4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0u;
    c.edx = GP4_GLOBAL(uint32_t, 0x0089151c);
    gp4::call_regs(0x00516daa, c);

    const uint32_t carry = (c.eax == 0x20000000u) ? gp4::flags::CF : 0u;
    const uint32_t flags = (gp4::flags::sub32(c.eax, 0x20000000u) & ~gp4::flags::CF) | carry;
    r->eax = c.eax;
    r->edx = c.edx;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00516df4, FUN_00516df4, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958+keys_006759c8")
