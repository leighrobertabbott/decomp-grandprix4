#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per arm so the compiler keeps both call addresses as written.
static __declspec(noinline) void LoadBindings(gp4::Regs& c) { gp4::call_regs(0x0052d8fa, c); }
static __declspec(noinline) void StoreBindings(gp4::Regs& c) { gp4::call_regs(0x0052d927, c); }

// Controls screen: resets the key-binding state (0x0052d8c1), calls the empty stub 0x00527f80 with eax = 0x2f9,
// then runs the screen loop 0x0053c8af on the static item list esi = 0x0084bc2a. Its result (eax, the selected
// entry) picks the action: nonzero loads the preset bindings (0x0052d8fa), zero stores the working copy
// (0x0052d927). One register block is threaded through every call; the flags are those of `or eax, eax` until the
// last callee overwrites them.
// FUNCTION: GP4 0x0052d897
void __cdecl FUN_0052d897(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0052d8c1, c);
    gp4::cdecl_<0x00527f80, void>();
    c.eax = 0x2f9u;
    c.esi = 0x0084bc2au;
    gp4::call_regs(0x0053c8af, c);

    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
    if (c.eax != 0u) {
        LoadBindings(c);
    } else {
        StoreBindings(c);
    }
    *r = c;
}
GP4_IMPL(0x0052d897, FUN_0052d897, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0+list_0052dba4+presets_007ab5b4+sel_007ab544; emu=skip:win32 PeekMessageA")
