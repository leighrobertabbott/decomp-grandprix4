#include <gp4/gp4.h>
#include <gp4/flags.h>

// With esi == 0 returns eax = 0 (flags of `cmp esi, 0`). Otherwise stores eax / edx / ecx in the dwords
// 0x00a69878 / 0x00a6987c / 0x00a69880, runs the compiled routine 0x0049eeb0 (which classifies that point and
// rewrites 0x00a69878) inside pushal/popal, and returns the dword 0x00a69878 in eax. popal does not restore
// the flags: they are those of the callee's last flag writer.
// FUNCTION: GP4 0x0051fb25
void __cdecl FUN_0051fb25(gp4::Regs* r) {
    if (r->esi == 0u) {
        r->eax = 0u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 0u));
        return;
    }

    GP4_GLOBAL(uint32_t, 0x00a69878) = r->eax;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = r->edx;
    GP4_GLOBAL(uint32_t, 0x00a69880) = r->ecx;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0049eeb0, c);

    r->eax = GP4_GLOBAL(uint32_t, 0x00a69878);
    r->eflags = c.eflags;
}
GP4_IMPL(0x0051fb25, FUN_0051fb25, "regs(eax:u32, ecx:u32, edx:u32, esi:u32{0,1}) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 GetLastError")
