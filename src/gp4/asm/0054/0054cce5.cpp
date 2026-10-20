#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal ... popal wrapper: saves eax, then (after the empty routine 0x00531b02) the registers
// ebx, ecx, edx and their doubles into six consecutive unaligned dwords at 0x00833cd7.. .
// All registers come back unchanged; the flags are those of the last `add edx, edx`.

// FUNCTION: GP4 0x0054cce5
void __cdecl FUN_0054cce5(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00833cd7) = r->eax;

    gp4::Regs c = *r;
    gp4::call_regs(0x00531b02, c);

    const uint32_t b = c.ebx;
    const uint32_t cx = c.ecx;
    const uint32_t d = c.edx;
    GP4_GLOBAL(uint32_t, 0x00833cdb) = b;
    GP4_GLOBAL(uint32_t, 0x00833cdf) = cx;
    GP4_GLOBAL(uint32_t, 0x00833ce3) = d;

    const uint32_t flags = gp4::flags::add32(d, d);
    GP4_GLOBAL(uint32_t, 0x00833ce7) = b + b;
    GP4_GLOBAL(uint32_t, 0x00833ceb) = cx + cx;
    GP4_GLOBAL(uint32_t, 0x00833cef) = d + d;

    r->eflags = gp4::flags::merge(c.eflags, flags);
}
GP4_IMPL(0x0054cce5, FUN_0054cce5, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32) -> (cf, pf, af, zf, sf, of)")
