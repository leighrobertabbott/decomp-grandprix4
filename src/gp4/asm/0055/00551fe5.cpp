#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares bx with 0x88. When they differ the flags of that compare are returned; when equal the
// routine re-tests eax (`or eax, eax`) and returns those flags instead. Registers are unchanged.
// FUNCTION: GP4 0x00551fe5
void __cdecl FUN_00551fe5(gp4::Regs* r) {
    const uint32_t cmpFlags = gp4::flags::sub16(r->ebx & 0xffffu, 0x88u);
    uint32_t flags = cmpFlags;
    if ((r->ebx & 0xffffu) == 0x88u) {
        flags = gp4::flags::logic32(r->eax | r->eax);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00551fe5, FUN_00551fe5, "regs(eax:u32, ebx:u32[0x80..0x90]) -> (cf, pf, af, zf, sf, of)")
