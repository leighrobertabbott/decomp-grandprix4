#include <gp4/gp4.h>

// `call 0x0053e202 ; call 0x00549e2c ; ret`: builds the dialog from the record in esi (0x0053e202),
// then posts the wake-up message and sleeps for 4 ms (0x00549e2c: 0x0046ae90 reaches the message
// pump). The registers and flags carry from the first call into the second and come back from the
// second.
// FUNCTION: GP4 0x0053e1f7
void __cdecl FUN_0053e1f7(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0053e202, c);
    gp4::call_regs(0x00549e2c, c);
    *r = c;
}
GP4_IMPL(0x0053e1f7, FUN_0053e1f7, "regs(esi:ptr[0x10]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
