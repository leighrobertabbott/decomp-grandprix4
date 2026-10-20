#include <gp4/gp4.h>

// pushal ... popal wrapper: for each of the 22 cars (esi = 0..21) whose flag byte in the table at
// 0x007c2a48 has bit 7 clear, runs 0x0043bc2f (itself a pushal/popal routine). All registers are
// restored by popal; the flags are those of the final `cmp esi, 0x16` (equal: ZF and PF set).
// FUNCTION: GP4 0x0043bcb8
void __cdecl FUN_0043bcb8(gp4::Regs* r) {
    for (uint32_t car = 0; car != 0x16u; ++car) {
        if (static_cast<int8_t>(GP4_ARRAY(uint8_t, 0x007c2a48)[car]) >= 0) {
            gp4::Regs callee = *r;
            callee.st_in = 0;
            callee.st_out = 0;
            callee.esi = car;
            gp4::call_regs(0x0043bc2f, callee);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043bcb8, FUN_0043bcb8, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
