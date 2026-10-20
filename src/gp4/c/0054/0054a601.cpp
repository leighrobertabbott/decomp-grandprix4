#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the byte at 0x007aafde to 2, advances the 3-bit counter with 0x0054ba6b (al = new value,
// flags of its `and al, 7`) and ends with `clc`: the flags are those of the callee's
// `and al, 7` with CF forced clear (already clear after a logic op).
// FUNCTION: GP4 0x0054a601
void __cdecl FUN_0054a601(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x007aafde) = 2u;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054ba6b, c);
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~gp4::flags::CF);
}
GP4_IMPL(0x0054a601, FUN_0054a601, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
