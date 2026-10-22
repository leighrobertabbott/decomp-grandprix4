#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds a text line at edi: pads the script at esi to 0x20 characters (0x0054c6f6), appends the script "=    "
// (0x00833b16), then prints the number the caller passed in eax (pushed on entry, popped back) with no padding
// (bh = 0) through the number formatter 0x0054642c, and finally appends the script at 0x00833b13 with the
// interpreter 0x0054d7c0. Registers and flags thread through the callees.
// FUNCTION: GP4 0x0054c67d
void __cdecl FUN_0054c67d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054c6f6, c);
    c.esi = 0x00833b16u;
    gp4::call_regs(0x0054d7c0, c);
    c.eax = r->eax;
    c.ebx = c.ebx & 0xffff00ffu;
    gp4::call_regs(0x0054642c, c);
    c.esi = 0x00833b13u;
    gp4::call_regs(0x0054d7c0, c);
    *r = c;
}
GP4_IMPL(0x0054c67d, FUN_0054c67d, "regs(eax:u32{0,1,5,9,10,99,100,12345,0xfffffffb,0xffffffff,0xffff0000,0x7fffffff,0x80000000,1000000}, ebx:u32, edx:u32, ebp:u32, esi:ptr[0x20]:u8{0,0x41,0x61,0x20,0x30,0x39}, edi:ptr[0x400]:bytes) -> (ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=items_0054d7c0")
