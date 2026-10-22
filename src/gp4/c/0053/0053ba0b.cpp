#include <gp4/gp4.h>
#include <gp4/flags.h>

// Under pushal/popal: expands the text scripts 0x707 and 5 one after the other into the static buffer 0x00823100
// (edi = 0x00823100, the interpreter 0x0054d798 advances edi past the first text), then restores every register
// and runs 0x0053c135 (esi = the menu item record) with them. Registers and flags are those 0x0053c135 leaves.
// FUNCTION: GP4 0x0053ba0b
void __cdecl FUN_0053ba0b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = 0x00823100u;
    c.esi = 0x707u;
    gp4::call_regs(0x0054d798, c);
    c.esi = 5u;
    gp4::call_regs(0x0054d798, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
    gp4::call_regs(0x0053c135, *r);
}
GP4_IMPL(0x0053ba0b, FUN_0053ba0b, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:ptr[0x20]:bytes, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_all_00835846+list_00549db9+pool_00549db9+textout_0054d211")
