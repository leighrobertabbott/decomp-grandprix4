#include <gp4/gp4.h>

// Advances the dword at 0x00826e32 by 0xc, then hands it (eax) and the dword at 0x00826e2e (ebx) to
// 0x0054cb7e, which keeps every register. The flags are the ones that routine leaves.
// FUNCTION: GP4 0x0052f1f0
void __cdecl FUN_0052f1f0(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00826e32) = GP4_GLOBAL(uint32_t, 0x00826e32) + 0xcu;
    gp4::Regs call = *r;
    call.ebx = GP4_GLOBAL(uint32_t, 0x00826e2e);
    call.eax = GP4_GLOBAL(uint32_t, 0x00826e32);
    gp4::call_regs(0x0054cb7e, call);
    r->eax = call.eax;
    r->ebx = call.ebx;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0052f1f0, FUN_0052f1f0, "regs() -> (eax, ebx, cf, pf, af, zf, sf, of); globals=layout=lim_0052f1f0")
