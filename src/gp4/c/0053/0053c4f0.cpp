#include <gp4/gp4.h>

// Menu pointer-event step, three register routines in a row: 0x0053daae (dispatches the current event
// to the item's handler), 0x0053db52 and 0x0053d58a. Each hands its registers to the next; registers and
// flags come back from the last call.
// FUNCTION: GP4 0x0053c4f0
void __cdecl FUN_0053c4f0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0053daae, c);
    gp4::call_regs(0x0053db52, c);
    gp4::call_regs(0x0053d58a, c);
    *r = c;
}
GP4_IMPL(0x0053c4f0, FUN_0053c4f0, "regs(eax:u32, ebx:u32, edx:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+state_0053d9a6+sel_0053d879+key_0053d58a+items_0053dc57+links_0053dca6+sel_0053d8c5")
