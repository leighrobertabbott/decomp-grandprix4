#include <gp4/gp4.h>

// esi = the selected car record [0x00679d30]; 0x00516d07 looks up its car number (byte +0x7c) in the order list
// 0x00675958 (eax = index, or 0 with CF set when absent) and the low word of the result goes to [0x00852d74].
// Registers and flags are those the lookup leaves.
// FUNCTION: GP4 0x005168bc
void __cdecl FUN_005168bc(gp4::Regs* r) {
    r->esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x00516d07, *r);
    GP4_GLOBAL(uint16_t, 0x00852d74) = static_cast<uint16_t>(r->eax);
}
GP4_IMPL(0x005168bc, FUN_005168bc, "regs() -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+carno_0066ff6c+order_00675958+state_0044334d")
