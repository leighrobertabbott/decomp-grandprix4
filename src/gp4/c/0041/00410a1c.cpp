#include <gp4/gp4.h>

// FUNCTION: GP4 0x00410a1c
// Per-wheel setup chain: seven calls on the same register file (ebx = setup bytes, ecx = wheel
// 0..3, esi = wheel record), each routine consumes what the one before left in the registers
// and flags: 0x4100ff, 0x410153, 0x41011a, 0x4100d8, 0x410154, 0x4105c7, 0x4108ac.
void __cdecl FUN_00410a1c(gp4::Regs* r) {
    gp4::call_regs(0x004100ff, *r);
    gp4::call_regs(0x00410153, *r);
    gp4::call_regs(0x0041011a, *r);
    gp4::call_regs(0x004100d8, *r);
    gp4::call_regs(0x00410154, *r);
    gp4::call_regs(0x004105c7, *r);
    gp4::call_regs(0x004108ac, *r);
}
GP4_IMPL(0x00410a1c, FUN_00410a1c, "regs(ebx:ptr[0x40]:layout=setup_00410a1c, ecx:u32[0..3], esi:ptr[0x410]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=wheel_00410a1c")
