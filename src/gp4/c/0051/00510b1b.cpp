#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the dword 0x00a53ad0 is nonzero: clears it, then runs 0x0051006c (eax = 2) and 0x00510309 (eax = 1)
// on the same register state. pushal/popal keep every register; the flags are those of `test dword, -1`
// when nothing ran and those the last callee leaves otherwise.
// FUNCTION: GP4 0x00510b1b
void __cdecl FUN_00510b1b(gp4::Regs* r) {
    const uint32_t pending = GP4_GLOBAL(uint32_t, 0x00a53ad0);
    const uint32_t tested = gp4::flags::merge(r->eflags, gp4::flags::logic32(pending));
    if (pending == 0u) {
        r->eflags = tested;
        return;
    }
    GP4_GLOBAL(uint32_t, 0x00a53ad0) = 0u;

    gp4::Regs c = *r;
    c.eflags = tested;
    c.eax = 2u;
    gp4::call_regs(0x0051006c, c);
    c.eax = 1u;
    gp4::call_regs(0x00510309, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00510b1b, FUN_00510b1b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
