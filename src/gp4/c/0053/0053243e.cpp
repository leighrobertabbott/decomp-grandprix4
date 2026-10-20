#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finishing-order pass over the 22 drivers and then the 11 pairs, in one fixed sequence of register
// routines that hand their register state to the next: 0x0054857e (per-driver points), 0x005486f7
// (pair sums), 0x00548602 (sort the drivers by score), 0x0054868c (tie marker), 0x0054873e (sort the
// pairs) and 0x005487cb (pair tie marker). Registers and flags are those of the last routine.
// FUNCTION: GP4 0x0053243e
void __cdecl FUN_0053243e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x0054857e, c);
    gp4::call_regs(0x005486f7, c);
    gp4::call_regs(0x00548602, c);
    gp4::call_regs(0x0054868c, c);
    gp4::call_regs(0x0054873e, c);
    gp4::call_regs(0x005487cb, c);
    *r = c;
}
GP4_IMPL(0x0053243e, FUN_0053243e, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040dfe9+tabs_0043ced2+words_0054888e+cnt_00548836+slots_005401a2")
