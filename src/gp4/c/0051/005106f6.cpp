#include <gp4/gp4.h>

// Looks up the record for index ecx in the 22-entry table at 0x0066fef0 (index clamped to 0..21
// by 0x0040838c) and returns its address in both eax and esi. The flags are whatever
// 0x0040838c leaves behind.
// FUNCTION: GP4 0x005106f6
void __cdecl FUN_005106f6(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.eax = r->ecx;
    gp4::call_regs(0x0040838c, call);
    r->eax = call.eax;
    r->esi = call.eax;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x005106f6, FUN_005106f6, "regs(ecx:i32[-8..40]) -> (eax, esi, cf, pf, af, zf, sf, of)")
