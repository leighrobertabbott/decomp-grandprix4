#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dispatches on the mode byte [0x007c2ea9] (loaded into al, so the low byte of eax keeps it): 0x80 runs
// the pushal/popal wrapper 0x00527aad, 0x44 runs 0x00527b0c; any other value does nothing. Both
// wrappers restore every register, so only the flags of their callee remain; with neither match the
// flags are those of the last `cmp al, 0x44`.
// FUNCTION: GP4 0x0053240a
void __cdecl FUN_0053240a(gp4::Regs* r) {
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    r->eax = (r->eax & 0xffffff00u) | mode;

    uint32_t arith = gp4::flags::sub8(mode, 0x80u);
    if (mode == 0x80u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x00527aad, c);
        arith = c.eflags;
    } else {
        arith = gp4::flags::sub8(mode, 0x44u);
        if (mode == 0x44u) {
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.eflags = gp4::flags::merge(c.eflags, arith);
            gp4::call_regs(0x00527b0c, c);
            arith = c.eflags;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053240a, FUN_0053240a, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040efa2")
