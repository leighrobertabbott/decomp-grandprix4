#include <gp4/gp4.h>

// `call 0x0054dedb ; call 0x0054defe ; ret`: first removes every newline from the text-edit record at
// edi (0x0054dedb), then word-wraps the text and trims it to five line breaks (0x0054defe). The
// registers each call leaves carry into the next; registers and flags come straight back from the
// calls. Same shape as the twins 0x00404961 / 0x0051730d.
// FUNCTION: GP4 0x0054ded0
void __cdecl FUN_0054ded0(gp4::Regs* r) {
    gp4::call_regs(0x0054dedb, *r);
    gp4::call_regs(0x0054defe, *r);
}
GP4_IMPL(0x0054ded0, FUN_0054ded0, "regs(edi:ptr[0x10]:layout=rec_0054defe, ebx:u32) -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=text_0054defe")
