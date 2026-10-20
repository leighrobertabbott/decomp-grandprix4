#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: stores the end record address [0x00852ccc] + [0x00852d20] * 0x160 in 0x00852cd4,
// then runs the interpolation 0x00556791 for side ebx = 0 and 1 (ebx is incremented from what the
// callee returns). popal restores every register; the flags are those of the final `cmp ebx, 2`.
// FUNCTION: GP4 0x0055693b
void __cdecl FUN_0055693b(gp4::Regs* r) {
    const uint32_t scaled = GP4_GLOBAL(uint32_t, 0x00852d20) * 0x160u;
    const uint32_t end = scaled + GP4_GLOBAL(uint32_t, 0x00852ccc);
    GP4_GLOBAL(uint32_t, 0x00852cd4) = end;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = end;
    c.ebx = 0u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(scaled, GP4_GLOBAL(uint32_t, 0x00852ccc)));
    do {
        gp4::call_regs(0x00556791, c);
        c.ebx += 1u;
    } while (c.ebx != 2u);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(c.ebx, 2u));
}
GP4_IMPL(0x0055693b, FUN_0055693b, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=multiply_0040ce7b+lerp_00556791")
