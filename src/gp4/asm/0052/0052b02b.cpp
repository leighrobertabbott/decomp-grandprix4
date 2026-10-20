#include <gp4/gp4.h>
#include <gp4/flags.h>

// Splits a day count (eax) into a calendar date: starting at year 1978 (0x7ba) the length of each year
// (0x0052b077, year in ecx) is subtracted while the remainder (ebx) is at least that long; then month dh =
// 1.. with the month lengths of 0x0052b056 (selector dh, year ecx). On return ecx = year, dh = month,
// dl = remaining days + 1, ebx = remaining days, eax = the last month length. The flags are those of the
// final `inc dl` with CF still set from the `cmp ebx, eax` that left the loop (ebx < eax).
// FUNCTION: GP4 0x0052b02b
void __cdecl FUN_0052b02b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = r->eax;
    c.ecx = 0x7bau;

    for (;;) {
        gp4::call_regs(0x0052b077, c);
        if (c.ebx < c.eax) {
            break;
        }
        ++c.ecx;
        c.ebx -= c.eax;
    }

    uint32_t month = 1u;
    for (;;) {
        c.edx = (c.edx & 0xffff00ffu) | (month << 8);
        gp4::call_regs(0x0052b056, c);
        if (c.ebx < c.eax) {
            break;
        }
        month = (month + 1u) & 0xffu;
        c.ebx -= c.eax;
    }

    const uint32_t day = c.ebx & 0xffu;
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = (c.edx & 0xffff00ffu) | (month << 8);
    r->edx = (r->edx & 0xffffff00u) | ((day + 1u) & 0xffu);
    r->eflags = gp4::flags::merge(r->eflags, (gp4::flags::add8(day, 1u) & ~gp4::flags::CF) | gp4::flags::CF);
}
GP4_IMPL(0x0052b02b, FUN_0052b02b, "regs(eax:u32[0..0x20000], edx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of)")
