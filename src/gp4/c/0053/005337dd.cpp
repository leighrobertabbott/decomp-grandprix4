#include <gp4/gp4.h>

// Stores the performance counter read by 0x0054e733 (edx:eax) in the qword at 0x007a8e04. eax and edx
// keep the counter halves; the other registers and the flags are unchanged by the callee.
// FUNCTION: GP4 0x005337dd
void __cdecl FUN_005337dd(gp4::Regs* r) {
    gp4::call_regs(0x0054e733, *r);
    GP4_GLOBAL(uint32_t, 0x007a8e04) = r->eax;
    GP4_GLOBAL(uint32_t, 0x007a8e08) = r->edx;
}
GP4_IMPL(0x005337dd, FUN_005337dd, "regs() -> (eax, edx); globals=fuzz:i32")
