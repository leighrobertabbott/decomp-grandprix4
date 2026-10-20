#include <gp4/gp4.h>

// `call 0x005484eb ; call 0x00545edb ; ret`: maps the driver byte in al to its slot index (0x005484eb),
// then writes the "first - second" text pair for that slot at [edi] (0x00545edb). The registers each
// call leaves carry into the next; registers and flags come back from the second call.
// FUNCTION: GP4 0x00545ed0
void __cdecl FUN_00545ed0(gp4::Regs* r) {
    gp4::Regs state = *r;
    gp4::call_regs(0x005484eb, state);
    gp4::call_regs(0x00545edb, state);
    *r = state;
}
GP4_IMPL(0x00545ed0, FUN_00545ed0, "regs(eax:u32[0..0x3f], ebx:u32, ecx:u32, edx:u32, edi:ptr[0x100]:bytes, ebp:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+drv22_0052d402")
