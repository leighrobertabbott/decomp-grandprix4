#include <gp4/gp4.h>

// pushal ... popal: every register comes back; the flags are those the last callee (0x00410c1b) left.
// Copies the three seed words 0x00629bca/cc/ce to the generator state 0x0066d8be/c0/c2, then runs the
// set-up chain 0x004112e3, 0x00404a33, 0x0041df3f, 0x0043ba31, 0x004446c2 and finally the two
// 7-byte routines 0x00410c46 (0x00601244 -> 0x0060124b) and 0x00410c1b (0x0060124b -> 0x00601251).
// FUNCTION: GP4 0x00408478
void __cdecl FUN_00408478(gp4::Regs* r) {
    GP4_GLOBAL(uint16_t, 0x0066d8be) = GP4_GLOBAL(uint16_t, 0x00629bca);
    GP4_GLOBAL(uint16_t, 0x0066d8c0) = GP4_GLOBAL(uint16_t, 0x00629bcc);
    GP4_GLOBAL(uint16_t, 0x0066d8c2) = GP4_GLOBAL(uint16_t, 0x00629bce);

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x004112e3, call);
    gp4::call_regs(0x00404a33, call);
    gp4::call_regs(0x0041df3f, call);
    gp4::call_regs(0x0043ba31, call);
    gp4::call_regs(0x004446c2, call);

    call.eax = 0x00601244u;
    call.edx = 0x0060124bu;
    gp4::call_regs(0x00410c46, call);

    call.eax = 0x0060124bu;
    call.edx = 0x00601251u;
    gp4::call_regs(0x00410c1b, call);

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00408478, FUN_00408478, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=grid_00628bc0+loops_0041df3f+codes_0043ba12+state_00444517")
