#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds a selection mask and runs the list-walk 0x0054eb10 with it in eax: all ones when the byte
// at 0x0082fd6c is nonzero, otherwise 0x40 when the byte at 0x007aafd3 is nonzero and 0 when not.
// eax is pushed and popped, so only the flags the callee leaves are visible.
// FUNCTION: GP4 0x0054eb70
void __cdecl FUN_0054eb70(gp4::Regs* r) {
    uint32_t mask;
    if (GP4_GLOBAL(uint8_t, 0x0082fd6c) == 0u) {
        mask = 0u;
        if (GP4_GLOBAL(uint8_t, 0x007aafd3) != 0u) {
            mask |= 0x40u;
        }
    } else {
        mask = 0xffffffffu;
    }
    gp4::Regs c = *r;
    c.eax = mask;
    gp4::call_regs(0x0054eb10, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054eb70, FUN_0054eb70, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_0054eaf7+links_0054eaf7+tail_0054eb10")
