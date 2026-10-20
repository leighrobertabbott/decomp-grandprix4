#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores 0x50 in the byte 0x00852dec when either of the bytes 0x007ab558 / 0x007ab559 is 2, else 0x20.
// al keeps the stored value (the rest of eax is untouched); the flags are those of the compare that
// decided: `cmp [0x7ab558], 2` when it was equal, else `cmp [0x7ab559], 2`.
// FUNCTION: GP4 0x00520007
void __cdecl FUN_00520007(gp4::Regs* r) {
    uint32_t al = 0x20u;
    const uint8_t first = GP4_GLOBAL(uint8_t, 0x007ab558);
    uint32_t flags = gp4::flags::sub8(first, 2u);
    if (first == 2u) {
        al = 0x50u;
    } else {
        const uint8_t second = GP4_GLOBAL(uint8_t, 0x007ab559);
        flags = gp4::flags::sub8(second, 2u);
        if (second == 2u) {
            al = 0x50u;
        }
    }
    GP4_GLOBAL(uint8_t, 0x00852dec) = static_cast<uint8_t>(al);
    r->eax = (r->eax & 0xffffff00u) | al;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00520007, FUN_00520007, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
