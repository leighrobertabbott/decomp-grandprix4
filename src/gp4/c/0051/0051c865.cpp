#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper: loads the three parameters (column offset eax, byte count ecx, delta edx) of
// the backward row copier 0x0051c80d from 0x817434/38/3c and runs it, then loads the parameters of
// its forward twin 0x0051c7c7 from 0x817440/44/48 and runs that on the same buffer edi. Every
// register comes back unchanged; the flags are those the second callee leaves.
// FUNCTION: GP4 0x0051c865
void __cdecl FUN_0051c865(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = GP4_GLOBAL(uint32_t, 0x00817434);
    c.ecx = GP4_GLOBAL(uint32_t, 0x00817438);
    c.edx = GP4_GLOBAL(uint32_t, 0x0081743c);
    gp4::call_regs(0x0051c80d, c);
    c.eax = GP4_GLOBAL(uint32_t, 0x00817440);
    c.ecx = GP4_GLOBAL(uint32_t, 0x00817444);
    c.edx = GP4_GLOBAL(uint32_t, 0x00817448);
    gp4::call_regs(0x0051c7c7, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051c865, FUN_0051c865, "regs(edi:ptr[0x2200]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=args_0051c865")
