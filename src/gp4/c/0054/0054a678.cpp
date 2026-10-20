#include <gp4/gp4.h>
#include <gp4/flags.h>

// Marks the byte 0x00824568 unused (0xff), copies the byte 0x007a97b2 to 0x00824567 (through al, so the
// low byte of eax holds it), advances the 3-bit counter with 0x0054ba6b (al = new value, flags of its
// `and al, 7`) and ends with `clc`: the flags are the callee's with CF forced clear.
// FUNCTION: GP4 0x0054a678
void __cdecl FUN_0054a678(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00824568) = 0xffu;
    const uint8_t value = GP4_GLOBAL(uint8_t, 0x007a97b2);
    GP4_GLOBAL(uint8_t, 0x00824567) = value;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (r->eax & 0xffffff00u) | value;
    gp4::call_regs(0x0054ba6b, c);
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~gp4::flags::CF);
}
GP4_IMPL(0x0054a678, FUN_0054a678, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ctr_0054a678")
