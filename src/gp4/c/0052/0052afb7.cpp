#include <gp4/gp4.h>
#include <gp4/flags.h>

// Day of the month: prints the byte [0x0081d36c] as a decimal number at the cursor edi (0x0054dac1), turns it
// into the text id esi = (day % 100) + 0x6e (0x0054d778) and expands that script at the new cursor through the
// interpreter 0x0054d798. Registers and flags are those the interpreter leaves.
// FUNCTION: GP4 0x0052afb7
void __cdecl FUN_0052afb7(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint8_t, 0x0081d36c);
    gp4::call_regs(0x0054dac1, c);
    gp4::call_regs(0x0054d778, c);
    gp4::call_regs(0x0054d798, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052afb7, FUN_0052afb7, "regs(ebx:u32, ecx:u32, edx:u32, esi:u32, edi:ptr[0x100]:bytes, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_all_00835846+textsel_00528c98")
