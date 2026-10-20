#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the menu wait loop 0x0052bd4f unless the link byte [0x007aafd3] is 0xff; in that case only the
// flags of `cmp byte [0x7aafd3], 0xff` change. The loop polls the message pump through 0x0053c8c8, so
// most cases end in PeekMessageA. Registers and flags come back from the callee.
// FUNCTION: GP4 0x0052bd40
void __cdecl FUN_0052bd40(gp4::Regs* r) {
    const uint8_t link = GP4_GLOBAL(uint8_t, 0x007aafd3);
    if (link == 0xffu) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(link, 0xffu));
        return;
    }
    gp4::call_regs(0x0052bd4f, *r);
}
GP4_IMPL(0x0052bd40, FUN_0052bd40, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
