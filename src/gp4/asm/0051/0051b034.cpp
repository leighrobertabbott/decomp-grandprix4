#include <gp4/gp4.h>
#include <gp4/flags.h>

// Flags-only test for ebx == 0xe3: always leaves the flags of `cmp ebx, 0xe3`, except that when
// ebx is 0xe3 the following `or eax, eax` replaces them with the logic flags of eax.
// FUNCTION: GP4 0x0051b034
void __cdecl FUN_0051b034(gp4::Regs* r) {
    uint32_t flags = gp4::flags::sub32(r->ebx, 0xe3u);
    if (r->ebx == 0xe3u) {
        flags = gp4::flags::logic32(r->eax);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051b034, FUN_0051b034, "regs(eax:i32[-2..2], ebx:u32[0xe0..0xe6]) -> (cf, pf, af, zf, sf, of)")
