#include <gp4/gp4.h>

// `call 0x0053873a ; call 0x00538771 ; ret`: resets the 0x400-entry table at 0x829cac (and copies the
// dword at 0x829868+0x14 to 0x82fcac), then stores the sum of the dwords at 0x829868+0x14/+0x10 to
// 0x82fcb0. Both callees restore the registers they use; the flags come back from the second call.
// FUNCTION: GP4 0x0053872f
void __cdecl FUN_0053872f(gp4::Regs* r) {
    gp4::call_regs(0x0053873a, *r);
    gp4::call_regs(0x00538771, *r);
}
GP4_IMPL(0x0053872f, FUN_0053872f, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
