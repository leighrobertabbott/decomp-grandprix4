#include <gp4/gp4.h>
#include <gp4/flags.h>

// Script reference: parses the decimal number at esi (0x0054d834, eax = the value, esi past the
// digits), skips the terminator byte, and runs the script interpreter 0x0054d798 with esi = the
// parsed string id (the interpreter writes its output at edi). esi is restored to the position after
// the terminator; the other registers and the flags are those the interpreter leaves.
// FUNCTION: GP4 0x0054d895
void __cdecl FUN_0054d895(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054d834, c);
    c.esi += 1u;
    const uint32_t next = c.esi;
    c.esi = c.eax;
    gp4::call_regs(0x0054d798, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = next;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054d895, FUN_0054d895, "regs(esi:ptr[0x40]:u8{=0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x20}, edi:ptr[0x100]:bytes) -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798")
