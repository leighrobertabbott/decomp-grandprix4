#include <gp4/gp4.h>
#include <gp4/flags.h>

// Kept out of line: MSVC otherwise merges the two calls of the if/else below into one call with a selected
// address, hiding both callees from the shape audit.
static __declspec(noinline) void run_00541ed2(gp4::Regs& c) { gp4::call_regs(0x00541ed2, c); }
static __declspec(noinline) void run_0054e615(gp4::Regs& c) { gp4::call_regs(0x0054e615, c); }

// Branches on the byte 0x007ad869 (the hardware-renderer byte): zero runs the wait loop 0x00541ed2 with eax =
// 0x100 (it polls the message pump); nonzero runs 0x0054e615 (the performance-counter stamp). Registers and
// flags come back as the chosen callee leaves them.
// FUNCTION: GP4 0x00532d2d
void __cdecl FUN_00532d2d(gp4::Regs* r) {
    const uint32_t flag = GP4_GLOBAL(uint8_t, 0x007ad869);
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(flag, 0u));
    if (flag == 0u) {
        c.eax = 0x100u;
        run_00541ed2(c);
    } else {
        run_0054e615(c);
    }
    *r = c;
}
GP4_IMPL(0x00532d2d, FUN_00532d2d, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=dw_00532d48; emu=skip:win32 PeekMessageA")
