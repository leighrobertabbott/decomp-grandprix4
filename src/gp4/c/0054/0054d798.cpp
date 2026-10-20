#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-script interpreter entry: pushal, push edi / push esi and call the empty stdcall routine 0x00509ec0
// (ret 8), popal; then esi = the string pointer of script id esi (0x0054d1ad, table 0x00835846) and the
// script expansion 0x0054d7c0 writes the text at edi. Registers and flags are those 0x0054d7c0 leaves.
// FUNCTION: GP4 0x0054d798
void __cdecl FUN_0054d798(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    const uint32_t args[2] = {c.esi, c.edi};
    gp4::call_regs_stdcall(0x00509ec0, c, args);
    gp4::call_regs(0x0054d1ad, *r);
    gp4::call_regs(0x0054d7c0, *r);
}
GP4_IMPL(0x0054d798, FUN_0054d798, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32{=0,1,2,3,5,6,7,0xba6,0xbad,0xbb1,0xbbb}, edi:ptr[0x100]:bytes, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798")
