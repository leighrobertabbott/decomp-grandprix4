#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / popal wrapper: runs the ring-record bit-clearing walk 0x00555c49 for user slots
// 0..0x1f except 0x17 and 0x18, with ebx = edi (the end record) and ecx = the slot. popal restores
// every register but not the flags, which end as the loop's final `cmp ecx, 0x20` on equal.
// FUNCTION: GP4 0x00555c2a
void __cdecl FUN_00555c2a(gp4::Regs* r) {
    const uint32_t endRecord = r->edi;
    for (uint32_t slot = 0u; slot != 0x20u; ++slot) {
        if (slot == 0x18u || slot == 0x17u) {
            continue;
        }
        gp4::Regs c = *r;
        c.ebx = endRecord;
        c.ecx = slot;
        gp4::call_regs(0x00555c49, c);
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x20u, 0x20u));
}
GP4_IMPL(0x00555c2a, FUN_00555c2a, "regs(edi:rec[0x8943a0,0x160,1..25]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=walk_00555c49")
