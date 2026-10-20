#include <gp4/gp4.h>

// Fills the eleven entries of the per-index tables DAT_0066d7d6 / DAT_0066d7ec (words, the pair
// returned by 0x0043cd21 for the index) and DAT_0066d802 (byte, the classification returned by
// 0x0043cc82), then runs 0x0043cbac and 0x0043ccae. pushal / popal keep every register; the flags
// are those the last callee leaves.
// FUNCTION: GP4 0x0043cd92
void __cdecl FUN_0043cd92(gp4::Regs* r) {
    gp4::Regs live = *r;
    live.st_in = 0;
    live.st_out = 0;

    for (uint32_t index = 0; index < 0xbu; ++index) {
        live.ecx = index;
        live.eax = index;
        gp4::call_regs(0x0043cd21, live);
        GP4_ARRAY(uint16_t, 0x0066d7d6)[index] = static_cast<uint16_t>(live.eax);
        GP4_ARRAY(uint16_t, 0x0066d7ec)[index] = static_cast<uint16_t>(live.edx);
        gp4::call_regs(0x0043cc82, live);
        GP4_ARRAY(uint8_t, 0x0066d802)[index] = static_cast<uint8_t>(live.eax);
    }

    live.ecx = 0xbu;
    gp4::call_regs(0x0043cbac, live);
    gp4::call_regs(0x0043ccae, live);

    r->eflags = (r->eflags & ~0x8d5u) | (live.eflags & 0x8d5u);
}
GP4_IMPL(0x0043cd92, FUN_0043cd92, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
