#include <gp4/gp4.h>

// `call 0x00548e2f ; call 0x005467ee ; ret`: decodes the first byte of the order list (flag in dl, index
// in eax, key table slot in esi), then runs the screen loop 0x005467ee, which polls 0x0053c8af and so
// reaches the message pump 0x00509b50 (PeekMessageA). Registers and flags carry from the first call
// into the second and come back from the second.
// FUNCTION: GP4 0x00533283
void __cdecl FUN_00533283(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00548e2f, c);
    gp4::call_regs(0x005467ee, c);
    *r = c;
}
GP4_IMPL(0x00533283, FUN_00533283, "regs(edx:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=order_00675958; emu=skip:win32 PeekMessageA")
