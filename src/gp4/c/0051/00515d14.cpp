#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores 1 in 0x008660b8 when the car record in esi is the second selected car [0x0066d828] (only
// while byte 0x007aafd4 is nonzero) or the first selected car [0x0066d820], else 0. eax is left as
// that value; the flags are those of `cmp eax, 1`. The `eax = 1` clamp after the `jle` is dead:
// eax is 0 or 1 here.

// FUNCTION: GP4 0x00515d14
void __cdecl FUN_00515d14(gp4::Regs* r) {
    const uint32_t car = r->esi;
    uint32_t flag = 0u;
    if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u && car == GP4_GLOBAL(uint32_t, 0x0066d828)) {
        flag = 1u;
    } else if (car == GP4_GLOBAL(uint32_t, 0x0066d820)) {
        flag = 1u;
    }
    GP4_GLOBAL(uint32_t, 0x008660b8) = flag;
    r->eax = flag;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(flag, 1u));
}
GP4_IMPL(0x00515d14, FUN_00515d14, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0040fa01")
