#include <gp4/gp4.h>
#include <gp4/flags.h>

// Waits for a special key: repeats the Windows message pump 0x00509b50 and the key-queue pop 0x0052d729
// (which reports through the carry flag that the popped word's high byte is zero, i.e. a plain character
// or an empty queue) and leaves when the carry is clear. eax and the flags are those of the last pop;
// the other registers are as the pump leaves them (it restores ebx, ebp, esi and edi).
// FUNCTION: GP4 0x0052d70d
void __cdecl FUN_0052d70d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    do {
        gp4::call_regs(0x00509b50, c);
        gp4::call_regs(0x0052d729, c);
    } while ((c.eflags & gp4::flags::CF) != 0u);
    *r = c;
}
GP4_IMPL(0x0052d70d, FUN_0052d70d, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0; emu=skip:win32 PeekMessageA")
