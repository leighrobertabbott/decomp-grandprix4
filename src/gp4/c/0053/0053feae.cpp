#include <gp4/gp4.h>
#include <gp4/flags.h>

// Calls the empty routine 0x00527f80 with eax = 0x303, runs the front-end poll 0x0053c8af with esi = 0x00848a8a
// (the menu record) and copies the dword [0x0083059f] (unaligned) to [0x008305a3] with eax saved around it, so
// the poll's result stays in eax. The poll waits through the message pump 0x00509b50 (via 0x0053d350) and so
// reaches PeekMessageA. Registers and flags come back as the poll leaves them.
// FUNCTION: GP4 0x0053feae
void __cdecl FUN_0053feae(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0x303u;
    gp4::call_regs(0x00527f80, c);
    c.esi = 0x00848a8au;
    gp4::call_regs(0x0053c8af, c);
    GP4_GLOBAL(uint32_t, 0x008305a3) = GP4_GLOBAL(uint32_t, 0x0083059f);
    *r = c;
}
GP4_IMPL(0x0053feae, FUN_0053feae, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
