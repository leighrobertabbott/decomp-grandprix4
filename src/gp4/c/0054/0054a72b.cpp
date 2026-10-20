#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the word at 0x007a97b2 (zero-extended) to the dword at 0x00822300, then advances the
// 3-bit counter with 0x0054ba6b (eax = the zero-extended word with its low byte replaced by the
// new counter value; flags of the callee's `and al, 7`) and ends with `clc`: the flags are the
// callee's with CF forced clear.
// FUNCTION: GP4 0x0054a72b
void __cdecl FUN_0054a72b(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint16_t, 0x007a97b2);
    GP4_GLOBAL(uint32_t, 0x00822300) = value;
    gp4::Regs c = *r;
    c.eax = value;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054ba6b, c);
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~gp4::flags::CF);
}
GP4_IMPL(0x0054a72b, FUN_0054a72b, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
