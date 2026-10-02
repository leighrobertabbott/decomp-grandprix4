#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040eaf7
void __cdecl FUN_0040eaf7(gp4::Regs* r) {
    // Saves ebx/ecx/edx/ebp/esi/edi around the 0x40ea46 routine, then eax = its ebx result.
    // Flags are those left by 0x40ea46.
    gp4::Regs saved = *r;
    gp4::call_regs(0x0040ea46, *r);
    saved.eax = r->ebx;
    saved.eflags = r->eflags;
    *r = saved;
}
GP4_IMPL(0x0040eaf7, FUN_0040eaf7, "regs(esi:ptr[0x41d]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
