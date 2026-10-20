#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the days from 1978 to a calendar date: ebx starts at the day (dl) - 1, then the lengths of the months
// before the month in dh are added one by one (0x0052b056 with the selector counting down from dh - 1 to 1,
// year in ecx), then the lengths of the years from ecx - 1 down to 1978 (0x0052b077, one `dec ecx` per year).
// eax = the total; ebx, ecx and edx are saved and restored. The flags are those of the final
// `cmp ecx, 0x7ba` that leaves the loop (ecx below 0x7ba).
// FUNCTION: GP4 0x0052affd
void __cdecl FUN_0052affd(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint32_t total = (r->edx & 0xffu) - 1u;
    uint32_t month = (r->edx >> 8) & 0xffu;
    uint32_t year = r->ecx;

    for (;;) {
        month = (month - 1u) & 0xffu;
        if (month == 0u) {
            break;
        }
        c.edx = (c.edx & 0xffff00ffu) | (month << 8);
        c.ecx = year;
        gp4::call_regs(0x0052b056, c);
        total += c.eax;
    }

    for (;;) {
        year = year - 1u;
        if (year < 0x7bau) {
            break;
        }
        c.ecx = year;
        gp4::call_regs(0x0052b077, c);
        total += c.eax;
    }

    r->eax = total;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(year, 0x7bau));
}
GP4_IMPL(0x0052affd, FUN_0052affd, "regs(ecx:u32[1970..2110], edx:u32{0x0101,0x0c1f,0x0201,0x0305,0x0000,0x00ff,0x0a0f,0x0b10,0x0d00}) -> (eax, cf, pf, af, zf, sf, of)")
