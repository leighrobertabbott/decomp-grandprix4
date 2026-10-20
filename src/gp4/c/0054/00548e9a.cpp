#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte 0x007ad8b2 is nonzero the two driver-flag counts of 0x0052ceef are added; a total
// above 1 is stored (unaligned dword 0x00831a65). Otherwise (byte zero, or total 0/1) the count of
// 0x00548ec5 is stored instead. eax = the stored value; edx is what 0x0052ceef left (unchanged when
// it did not run); the flags are those of `cmp eax, 1` (total above 1) or of 0x00548ec5.
// FUNCTION: GP4 0x00548e9a
void __cdecl FUN_00548e9a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    bool fallback = true;
    if (GP4_GLOBAL(uint8_t, 0x007ad8b2) != 0u) {
        gp4::call_regs(0x0052ceef, c);
        c.eax = c.eax + c.edx;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 1u));
        if (c.eax > 1u) {
            fallback = false;
        }
    }
    if (fallback) {
        gp4::call_regs(0x00548ec5, c);
    }
    GP4_GLOBAL(uint32_t, 0x00831a65) = c.eax;
    r->eax = c.eax;
    r->edx = c.edx;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00548e9a, FUN_00548e9a, "regs(edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv_005117d3+tabs_0043ced2+scan_00548e9a")
