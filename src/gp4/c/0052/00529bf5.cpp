#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper that refreshes the per-node flag bits of the two chains hanging off the block at
// 0x00840fd2. First FUN_0053e58c gets the 96-bit mask ebx:edx:eax (esi = 0x840fd2): all zero when
// bit 7 of the byte 0x0081d22a is set, else 0x492:0x49249249:0x24924800; eax |= 3 when the byte
// 0x007a8e0f is 0xff, otherwise ebx |= 0x8800. Then FUN_0053d1f4 gets edx:eax = 0:0x48 (0:0 when bit 7
// is set). popal restores every register; the flags that escape are those left by FUN_0053d1f4.
// FUNCTION: GP4 0x00529bf5
void __cdecl FUN_00529bf5(gp4::Regs* r) {
    uint32_t low = 0u;
    uint32_t high = 0u;
    uint32_t top = 0u;
    if ((GP4_GLOBAL(uint8_t, 0x0081d22a) & 0x80u) == 0u) {
        low = 0x24924800u;
        high = 0x49249249u;
        top = 0x492u;
    }
    if (GP4_GLOBAL(uint8_t, 0x007a8e0f) == 0xffu) {
        low |= 3u;
    } else {
        top |= 0x8800u;
    }
    gp4::Regs first = *r;
    first.eax = low;
    first.edx = high;
    first.ebx = top;
    first.esi = 0x00840fd2u;
    gp4::call_regs(0x0053e58c, first);

    gp4::Regs second = *r;
    second.eax = 0u;
    second.edx = 0u;
    if ((GP4_GLOBAL(uint8_t, 0x0081d22a) & 0x80u) == 0u) {
        second.eax = 0x48u;
    }
    second.esi = 0x00840fd2u;
    gp4::call_regs(0x0053d1f4, second);
    r->eflags = gp4::flags::merge(r->eflags, second.eflags);
}
GP4_IMPL(0x00529bf5, FUN_00529bf5, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=sel_00529bf5")
