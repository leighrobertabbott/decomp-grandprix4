#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the byte 0x00831a97; when the byte 0x007ad8b2 is nonzero, adds the two driver-flag counts
// of 0x0052ceef (eax += edx) and sets 0x00831a97 = 0xff when the total is 2 or more. eax = the total
// and edx = the second count in that case (both unchanged otherwise); the flags are those of
// `cmp byte [0x7ad8b2], 0` or `cmp eax, 2`.
// FUNCTION: GP4 0x005484a8
void __cdecl FUN_005484a8(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00831a97) = 0u;
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007ad8b2);
    uint32_t flags = gp4::flags::sub8(mode, 0u);
    if (mode != 0u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x0052ceef, c);
        const uint32_t total = c.eax + c.edx;
        flags = gp4::flags::sub32(total, 2u);
        if (total >= 2u) {
            GP4_GLOBAL(uint8_t, 0x00831a97) = 0xffu;
        }
        r->eax = total;
        r->edx = c.edx;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005484a8, FUN_005484a8, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv_005117d3")
