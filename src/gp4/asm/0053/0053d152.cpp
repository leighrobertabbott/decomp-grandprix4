#include <gp4/gp4.h>

// Copies esi into edx, then calls the empty routine 0x00531b35. Only edx changes.
// FUNCTION: GP4 0x0053d152
void __cdecl FUN_0053d152(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    r->edx = esi;                               // mov edx, esi
    gp4::cdecl_<0x00531b35, void>();            // call 0x531b35 (a bare `ret`)
}
GP4_IMPL(0x0053d152, FUN_0053d152, "regs(esi:u32) -> (edx)")
