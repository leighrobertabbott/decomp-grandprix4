#include <gp4/gp4.h>
#include <gp4/flags.h>

// Days in the year ecx: 366 (0x16e) for a multiple of 4 that is not a multiple of 100, else 365 (0x16d).
// ebx/edx are saved and restored. The division is "cdq; div ebx" (ebx = 100), so ecx must be below 2^31
// when it is a multiple of 4 (a negative one overflows the quotient and faults the original). The
// flags are those of the last flag writer: "and eax, 3", or "or edx, edx" when the year is a multiple of 4.
// FUNCTION: GP4 0x0052b077
void __cdecl FUN_0052b077(gp4::Regs* r) {
    const uint32_t year = r->ecx;
    uint32_t arith = gp4::flags::logic32(year & 3u);
    uint32_t days = 0x16du;
    if ((year & 3u) == 0u) {
        const uint32_t remainder = year % 100u;
        arith = gp4::flags::logic32(remainder);
        if (remainder != 0u) {
            days = 0x16eu;
        }
    }
    r->eax = days;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052b077, FUN_0052b077, "regs(ecx:u32{=0,1,2,3,4,5,8,12,96,100,101,104,200,300,400,404,1000,1896,1900,1996,2000,2001,2002,2004,2100,2400,4000,0x7ffffffc,0x7fffff9c,0x12345678}) -> (eax, cf, pf, af, zf, sf, of)")
