#include <gp4/gp4.h>

// Loads eax / ecx / edx / esi from the four dwords 0x006278c0..0x006278cc and calls 0x0051fb25 with them. ebx, ecx, edx,
// ebp, esi and edi are pushed and popped around the call, so only eax and the flags the callee leaves remain.
// FUNCTION: GP4 0x0041d96d
void __cdecl FUN_0041d96d(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = GP4_GLOBAL(uint32_t, 0x006278c0);
    call.ecx = GP4_GLOBAL(uint32_t, 0x006278c4);
    call.edx = GP4_GLOBAL(uint32_t, 0x006278c8);
    call.esi = GP4_GLOBAL(uint32_t, 0x006278cc);
    gp4::call_regs(0x0051fb25, call);
    r->eax = call.eax;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0041d96d, FUN_0041d96d, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
