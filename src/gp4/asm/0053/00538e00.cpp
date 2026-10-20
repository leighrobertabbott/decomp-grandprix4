#include <gp4/gp4.h>
#include <gp4/flags.h>

// When edx is 5 or 6 clears dword 0x0082fd88; then edx = 0. The flags are those of the last compare:
// `cmp edx, 5` when edx is 5, otherwise `cmp edx, 6`.
// FUNCTION: GP4 0x00538e00
void __cdecl FUN_00538e00(gp4::Regs* r) {
    const uint32_t selector = r->edx;
    uint32_t flags;
    if (selector == 5u) {
        flags = gp4::flags::sub32(selector, 5u);
    } else {
        flags = gp4::flags::sub32(selector, 6u);
    }
    if (selector == 5u || selector == 6u) {
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
    }
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00538e00, FUN_00538e00, "regs(edx:u32{5,6,0,7,4}) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
