#include <gp4/gp4.h>
#include <gp4/flags.h>

// `mov ebx, 0x824566 ; mov ecx, 1 ; call 0x0054b6ff ; mov dl, 0x1a ; call 0x0054b10c ; ret`: the one-byte
// send of the front-end link layer: al is chosen by 0x0054b6ff, then 0x0054b10c sends the buffer
// ebx (0x00824566, one byte) of length ecx = 1 with the packet type dl = 0x1a (it polls the message
// pump). Registers and flags come back as 0x0054b10c leaves them.
// FUNCTION: GP4 0x0054b6be
void __cdecl FUN_0054b6be(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = 0x00824566u;
    c.ecx = 1u;
    gp4::call_regs(0x0054b6ff, c);
    c.edx = (c.edx & 0xffffff00u) | 0x1au;
    gp4::call_regs(0x0054b10c, c);
    *r = c;
}
GP4_IMPL(0x0054b6be, FUN_0054b6be, "regs(eax:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
