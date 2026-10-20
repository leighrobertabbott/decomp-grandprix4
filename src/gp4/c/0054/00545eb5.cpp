#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-script wrapper: pushes eax, runs the driver-slot lookup 0x005484eb on it, hands the result to the
// text-id wrapper 0x00545ec2 (esi = eax + 0xba6, text interpreter), and pops eax again. Every other register
// and the flags are those the last callee leaves.
// FUNCTION: GP4 0x00545eb5
void __cdecl FUN_00545eb5(gp4::Regs* r) {
    const uint32_t saved_eax = r->eax;
    gp4::Regs c = *r;
    gp4::call_regs(0x005484eb, c);
    gp4::call_regs(0x00545ec2, c);
    *r = c;
    r->eax = saved_eax;
}
GP4_IMPL(0x00545eb5, FUN_00545eb5, "regs(eax:u32[0..0x3f], ebx:u32, ecx:u32, edx:u32, edi:ptr[0x100]:bytes, ebp:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+drv22_0052d402")
