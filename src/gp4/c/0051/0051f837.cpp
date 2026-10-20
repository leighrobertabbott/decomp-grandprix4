#include <gp4/gp4.h>
#include <gp4/flags.h>

// `push ebx/ecx/edx/ebp/esi/edi ; call 0x0052a279 ; pop ... ; ret`: the record-array entry address
// lookup with every register except eax preserved around it. eax is the address, the flags are those
// of the callee's final `add`.
// FUNCTION: GP4 0x0051f837
void __cdecl FUN_0051f837(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0052a279, c);
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f837, FUN_0051f837, "regs(eax:u32, ebx:u32, ecx:u32) -> (eax, cf, pf, af, zf, sf, of)")
