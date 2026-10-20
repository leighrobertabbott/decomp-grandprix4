#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the menu/session state bytes: [0x007aafdc] = 0 and [0x0082fd6c] = 1, then the list walk
// 0x0054eb70 runs (with the selection mask all ones because [0x0082fd6c] is set), then
// [0x007aafda], [0x007aafe0], [0x007aafe1], [0x007aafd0], [0x007aafd9], [0x0083318a] and
// [0x007aafd8] are cleared and [0x0082456a] = 0xff. The stores leave the flags alone: they are
// those of the callee.
// FUNCTION: GP4 0x0053577b
void __cdecl FUN_0053577b(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x007aafdc) = 0u;
    GP4_GLOBAL(uint8_t, 0x0082fd6c) = 1u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054eb70, c);

    GP4_GLOBAL(uint8_t, 0x007aafda) = 0u;
    GP4_GLOBAL(uint8_t, 0x007aafe0) = 0u;
    GP4_GLOBAL(uint8_t, 0x007aafe1) = 0u;
    GP4_GLOBAL(uint8_t, 0x007aafd0) = 0u;
    GP4_GLOBAL(uint8_t, 0x007aafd9) = 0u;
    GP4_GLOBAL(uint8_t, 0x0082456a) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x0083318a) = 0u;
    GP4_GLOBAL(uint8_t, 0x007aafd8) = 0u;

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053577b, FUN_0053577b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_0054eaf7+links_0054eaf7+tail_0054eb10")
