#include <gp4/gp4.h>

// push ebx/ecx/edx/ebp/esi/edi ... pop wrapper: runs the 0x0040fb09 selection for the record in
// esi, then returns the pointer global 0x00601f24 in eax. Every register but eax is restored;
// the flags are those of the last flag writer inside the callee.
// FUNCTION: GP4 0x0040fb30
void __cdecl FUN_0040fb30(gp4::Regs* r) {
    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x0040fb09, callee);

    r->eax = GP4_GLOBAL(uint32_t, 0x00601f24);
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x0040fb30, FUN_0040fb30, "regs(esi:ptr[0x84]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
