#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the dword at the unaligned address 0x00828293 to 0x8e, runs the front-end poll 0x0053c8af on the menu
// record esi = 0x00852837 and then the layout pass 0x0053420b. One register block is threaded through both
// calls; registers and flags come back from the second. The poll waits through the message pump and so
// reaches PeekMessageA.
// FUNCTION: GP4 0x00534bbb
void __cdecl FUN_00534bbb(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00828293) = 0x8eu;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = 0x00852837u;
    gp4::call_regs(0x0053c8af, c);
    gp4::call_regs(0x0053420b, c);
    *r = c;
}
GP4_IMPL(0x00534bbb, FUN_00534bbb, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
