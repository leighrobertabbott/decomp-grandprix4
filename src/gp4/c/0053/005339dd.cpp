#include <gp4/gp4.h>

// Three register routines in a row, the field draw (text field) 0x00533e11, 0x00533e4b and 0x00533faa (the last inserts the field's
// rectangle into the draw list with 0x00549db9 and runs the update step 0x00549e2c). Registers and flags
// come back from the last call.
// FUNCTION: GP4 0x005339dd
void __cdecl FUN_005339dd(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00533e11, c);
    gp4::call_regs(0x00533e4b, c);
    gp4::call_regs(0x00533faa, c);
    *r = c;
}
GP4_IMPL(0x005339dd, FUN_005339dd, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+state_0053d9a6+span_00533dc5+font_005339ed+chars_00828108+list_00549db9; emu=skip:win32 PeekMessageA")
