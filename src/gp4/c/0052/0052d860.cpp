#include <gp4/gp4.h>
#include <gp4/flags.h>

// Screen loop of the controls menu: calls the empty stub 0x00527f80 (eax = 0x2fa), clears the item
// flags of the static list esi = 0x0084bcea with 0x0053e4db (eax = 2 when the selected preset byte
// [0x007ab544] is below 4, else 0), then runs the screen 0x0053c8af. While it answers 1 the screen
// 0x0052d897 runs and the loop starts again; any other answer ends it. One register block is
// threaded through every call; the flags are those of `cmp eax, 1`.
// FUNCTION: GP4 0x0052d860
void __cdecl FUN_0052d860(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    for (;;) {
        gp4::cdecl_<0x00527f80, void>();
        c.eax = (GP4_GLOBAL(uint8_t, 0x007ab544) < 4u) ? 2u : 0u;
        c.esi = 0x0084bcea;
        gp4::call_regs(0x0053e4db, c);
        gp4::call_regs(0x0053c8af, c);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 1u));
        if (c.eax != 1u) {
            break;
        }
        gp4::call_regs(0x0052d897, c);
    }
    *r = c;
}
GP4_IMPL(0x0052d860, FUN_0052d860, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0+list_0052dba4+presets_007ab5b4+sel_007ab544; emu=skip:win32 PeekMessageA")
