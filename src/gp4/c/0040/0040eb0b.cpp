#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040eb0b
void __cdecl FUN_0040eb0b(gp4::Regs* r) {
    // Saves ebx/ecx/edx/ebp/esi/edi around the 0x40ea09 routine, then eax = its ebx result.
    // Flags are those left by 0x40ea09.
    gp4::Regs saved = *r;
    gp4::call_regs(0x0040ea09, *r);
    saved.eax = r->ebx;
    saved.eflags = r->eflags;
    *r = saved;
}
GP4_IMPL(0x0040eb0b, FUN_0040eb0b, "regs(esi:ptr[0x41d]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
