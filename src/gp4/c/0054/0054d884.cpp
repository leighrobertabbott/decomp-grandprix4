#include <gp4/gp4.h>
#include <gp4/flags.h>

// Script reference with a fixed id: skips the run of digits at esi (0x0054d834, which always leaves eax = 0),
// steps esi past the terminator byte, turns eax into the script id esi = (eax % 100) + 0x45 (0x0054d763, so
// the id is always 0x45) and runs the script interpreter 0x0054d798 on it (its output goes to edi). The
// registers and flags are those the interpreter leaves.
// FUNCTION: GP4 0x0054d884
void __cdecl FUN_0054d884(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054d834, c);
    c.esi += 1u;
    gp4::call_regs(0x0054d763, c);
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
GP4_IMPL(0x0054d884, FUN_0054d884, "regs(esi:ptr[0x40]:u8{=0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x20}, edi:ptr[0x100]:bytes) -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_all_00835846")
