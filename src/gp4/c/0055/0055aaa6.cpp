#include <gp4/gp4.h>
#include <gp4/flags.h>

// Randomises the start state of the 0x420-spaced records: initialises records 0..15 through 0x0055aae1, then fills
// the eleven dwords at 0x8914a0 with 9 random bits each, taken from two successive draws of 0x004015dc (first draw
// supplies the low byte, the second draw the bit above it). pushal/popal restores every register; the flags are those
// of the final `cmp ebx, 0xb`.
// FUNCTION: GP4 0x0055aaa6
void __cdecl FUN_0055aaa6(gp4::Regs* r) {
    gp4::Regs c = *r;

    for (uint32_t i = 0; i != 0x10u; ++i) {
        c.ebx = i;
        gp4::call_regs(0x0055aae1, c);
    }

    for (uint32_t i = 0; i != 0xbu; ++i) {
        c.ebx = i;
        gp4::call_regs(0x004015dc, c);
        const uint32_t first = c.eax;
        gp4::call_regs(0x004015dc, c);
        const uint32_t value = (((c.eax << 8) & 0xffffff00u) | (first & 0xffu)) & 0x1ffu;
        c.edx = first;
        c.eax = value;
        GP4_ARRAY(uint32_t, 0x008914a0)[i] = value;
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0xbu, 0xbu));
}
GP4_IMPL(0x0055aaa6, FUN_0055aaa6, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0040cb38+state_0062d1b4+flags_0055aae1")
