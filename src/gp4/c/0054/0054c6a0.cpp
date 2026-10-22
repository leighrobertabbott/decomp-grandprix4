#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds a text line at edi: pads the script at esi to 0x20 characters (0x0054c6f6), appends the script "=    "
// (0x00833b16), then the script whose pointer the caller passed in eax (pushed on entry, popped into esi), then
// the script at 0x00833b13, each expanded by the interpreter 0x0054d7c0. Registers and flags thread through the
// callees; esi and edi come back advanced.
// FUNCTION: GP4 0x0054c6a0
void __cdecl FUN_0054c6a0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054c6f6, c);
    c.esi = 0x00833b16u;
    gp4::call_regs(0x0054d7c0, c);
    c.esi = r->eax;
    gp4::call_regs(0x0054d7c0, c);
    c.esi = 0x00833b13u;
    gp4::call_regs(0x0054d7c0, c);
    *r = c;
}
GP4_IMPL(0x0054c6a0, FUN_0054c6a0, "regs(eax:ptr[0x40]:u8{0,0,1,2,3,4,5,0x13,0x14,0x30,0x31,0x41,0x42}, ebx:u32, edx:u32, ebp:u32, esi:ptr[0x20]:u8{0,0x41,0x61,0x20,0x30,0x39}, edi:ptr[0x400]:bytes) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=items_0054d7c0")
