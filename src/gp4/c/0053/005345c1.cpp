#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the three-stage control check 0x005345dc unless one of the bytes 0x007ab17e / 0x00828168 is
// 0xff (not set); in that case it fails at once. CF = 1 on failure (`stc` keeps the other flags of the
// last compare), otherwise the flags are those of the check. All registers are kept.
// FUNCTION: GP4 0x005345c1
void __cdecl FUN_005345c1(gp4::Regs* r) {
    const uint32_t first = GP4_GLOBAL(uint8_t, 0x007ab17e);
    uint32_t arith = gp4::flags::sub8(first, 0xffu);
    if (first != 0xffu) {
        const uint32_t second = GP4_GLOBAL(uint8_t, 0x00828168);
        arith = gp4::flags::sub8(second, 0xffu);
        if (second != 0xffu) {
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.eflags = gp4::flags::merge(c.eflags, arith);
            gp4::call_regs(0x005345dc, c);
            r->eflags = gp4::flags::merge(r->eflags, c.eflags);
            return;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith | static_cast<uint32_t>(gp4::flags::CF));
}
GP4_IMPL(0x005345c1, FUN_005345c1, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ctl_007ab548+mask_005345c1")
