#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sends the 8 bytes at 0x0066d8be..0x0066d8c6 through the link-layer sender 0x0054b10c: ebx = the buffer,
// ecx = its length (0x0066d8c6 - 0x0066d8be, computed with a `sub` whose flags carry on), dl = the packet type
// 0x1d and al = 0 (the upper bits of eax and edx are kept). The sender polls the message pump. Registers and
// flags come back as 0x0054b10c leaves them.
// FUNCTION: GP4 0x0054b686
void __cdecl FUN_0054b686(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = 0x0066d8beu;
    c.ecx = 0x0066d8c6u - 0x0066d8beu;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(0x0066d8c6u, 0x0066d8beu));
    c.edx = (c.edx & 0xffffff00u) | 0x1du;
    c.eax = c.eax & 0xffffff00u;
    gp4::call_regs(0x0054b10c, c);
    *r = c;
}
GP4_IMPL(0x0054b686, FUN_0054b686, "regs(eax:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
