#include <gp4/gp4.h>
#include <gp4/flags.h>

// Month name: esi = [0x0081d36d] + 0x2d when the byte [0x0081d36e] is 0xff, else + 0x21, and the text script with
// that id is expanded at edi by the interpreter 0x0054d798. Registers and flags are those the interpreter leaves.
// FUNCTION: GP4 0x0052afce
void __cdecl FUN_0052afce(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint32_t id = GP4_GLOBAL(uint8_t, 0x0081d36d);
    if (GP4_GLOBAL(uint8_t, 0x0081d36e) == 0xffu) {
        id += 0x2du;
    } else {
        id += 0x21u;
    }
    c.esi = id;
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
GP4_IMPL(0x0052afce, FUN_0052afce, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, edi:ptr[0x100]:bytes, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_all_00835846+textsel_00528c98")
