#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the flags: `cmp ebx, 10`; when ebx == 10 they are replaced by those of `or eax, eax`
// (CF = OF = AF = 0, ZF/SF/PF from eax). Only eflags changes.
// FUNCTION: GP4 0x00551809
void __cdecl FUN_00551809(gp4::Regs* r) {
    const uint32_t eax = r->eax;
    const uint32_t ebx = r->ebx;
    uint32_t f = gp4::flags::sub32(ebx, 0x0au);              // cmp ebx, 0xa
    if (ebx == 0x0au) {
        f = gp4::flags::logic32(eax | eax);                  // or eax, eax
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x00551809, FUN_00551809, "regs(eax:u32, ebx:u32[0..15]) -> (cf, pf, af, zf, sf, of)")
