#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: copies the random-state bytes (0x004015e8), steps the generator (0x004015dc, which
// takes eax and returns the next byte in al) and stores that byte, zero-extended, to 0x00a6987c.
// popal restores every GPR; the flags are those of the final `and eax, 0xff`.
// FUNCTION: GP4 0x0051f9b0
void __cdecl FUN_0051f9b0(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x004015e8, c);
    gp4::call_regs(0x004015dc, c);
    const uint32_t value = c.eax & 0xffu;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x0051f9b0, FUN_0051f9b0, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
