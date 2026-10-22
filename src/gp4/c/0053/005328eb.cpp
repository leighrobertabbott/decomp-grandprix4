#include <gp4/gp4.h>

// Calls the empty hook 0x005ae990 under pushal/popal (a bare ret, so nothing changes) and then tail-calls 0x0054fb87.
// Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x005328eb
void __cdecl FUN_005328eb(gp4::Regs* r) {
    gp4::cdecl_<0x005ae990, void>();
    gp4::call_regs(0x0054fb87, *r);
}
GP4_IMPL(0x005328eb, FUN_005328eb, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+surfcopy_00549d55+frame_0082fd70+case_0054fd4d+list_00549db9+tbl_0054fbfb+pop_005328eb; wall=3600")
