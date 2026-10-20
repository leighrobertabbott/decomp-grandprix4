#include <gp4/gp4.h>

// Sets the byte 0x0062ad78 (0x80: the lap-fraction estimate uses the word 0x007c2e96) and schedules the next
// time-limited event through 0x004424ef. That routine is a pushal / popal wrapper, so only the flags it leaves remain.
// FUNCTION: GP4 0x004424d5
void __cdecl FUN_004424d5(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0062ad78) = 0x80u;
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x004424ef, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x004424d5, FUN_004424d5, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958+cars_0066fef0+lap_0043b482+carno_0066ff6c+state_0040d8a5")
