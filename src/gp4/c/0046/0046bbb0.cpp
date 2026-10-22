#include <gp4/gp4.h>

// Base-class vtable store: writes the vtable pointer 0x005f0758 at [ecx] and
// returns without touching eax (callers read the eax they passed in).
// FUNCTION: GP4 0x0046bbb0
void __cdecl FUN_0046bbb0(gp4::Regs* r) {
    uint32_t* self = (uint32_t*)r->ecx;
    *self = 0x005f0758;
}
GP4_IMPL(0x0046bbb0, FUN_0046bbb0, "regs(ecx:ptr[0x100]:bytes, eax:u32) -> (eax)")
