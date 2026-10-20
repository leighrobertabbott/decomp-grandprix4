#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper that builds the flag word for the item chain walk 0x0053e4db (head 0x0084e9bb):
// ebx = 0xb when bit [0x0081d1d4] of the mask byte [0x0081d1d7] is set (`1 << cl` is an 8-bit shift), else 0;
// eax = ebx, plus 0x1dc00 when the byte [0x0081d1d6] is nonzero and plus 0x20000 when the byte [0x0081d1e9]
// is zero. popal restores every register, so only the flags 0x0053e4db leaves survive.
// FUNCTION: GP4 0x00528927
void __cdecl FUN_00528927(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.esi = 0x0084e9bbu;
    c.ebx = 0u;
    const uint8_t shift = GP4_GLOBAL(uint8_t, 0x0081d1d4);
    const uint8_t bit = static_cast<uint8_t>(1u << (shift & 0x1fu));
    const uint8_t hit = static_cast<uint8_t>(bit & GP4_GLOBAL(uint8_t, 0x0081d1d7));
    if (hit != 0u) {
        c.ebx = 0x0bu;
    }
    c.eax = c.ebx;
    if (GP4_GLOBAL(uint8_t, 0x0081d1d6) != 0u) {
        c.eax |= 0x1dc00u;
    }
    if (GP4_GLOBAL(uint8_t, 0x0081d1e9) == 0u) {
        c.eax |= 0x20000u;
    }
    gp4::call_regs(0x0053e4db, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00528927, FUN_00528927, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=menu_00529408+mask_0081d1d7")
