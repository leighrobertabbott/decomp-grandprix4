#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the byte 0x007a97b2 to 0x007a93c4 (through al, so the low byte of eax holds it), advances
// the 3-bit counter with 0x0054ba6b (al = new value, flags of its `and al, 7`) and ends with `clc`:
// the flags are those of the callee's `and al, 7` with CF forced clear.
// FUNCTION: GP4 0x0054a6c6
void __cdecl FUN_0054a6c6(gp4::Regs* r) {
    const uint8_t value = GP4_GLOBAL(uint8_t, 0x007a97b2);
    GP4_GLOBAL(uint8_t, 0x007a93c4) = value;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (r->eax & 0xffffff00u) | value;
    gp4::call_regs(0x0054ba6b, c);
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~gp4::flags::CF);
}
GP4_IMPL(0x0054a6c6, FUN_0054a6c6, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
