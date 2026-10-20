#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054a6a1
// Copies the 9-byte block 0x007a97b2 (dword, dword, byte) to 0x008270be, then advances the 3-bit counter
// with 0x0054ba6b (eax = the second dword with its low byte replaced by the copied byte on entry; al =
// the new counter value on return) and ends with `clc`: the flags are those of the callee's `and al, 7`
// with CF forced clear.
void __cdecl FUN_0054a6a1(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x008270be) = GP4_GLOBAL(uint32_t, 0x007a97b2);
    const uint32_t middle = GP4_GLOBAL(uint32_t, 0x007a97b6);
    GP4_GLOBAL(uint32_t, 0x008270c2) = middle;
    const uint8_t last = GP4_GLOBAL(uint8_t, 0x007a97ba);
    GP4_GLOBAL(uint8_t, 0x008270c6) = last;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (middle & 0xffffff00u) | last;
    gp4::call_regs(0x0054ba6b, c);
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~gp4::flags::CF);
}
GP4_IMPL(0x0054a6a1, FUN_0054a6a1, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ctr_0054a678")
