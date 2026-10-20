#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper that only compares: `cmp byte [0x7c2e82], 0xff`, and when that is not equal also
// `cmp byte [0x7c2ea9], 0x80`. Both conditional jumps land on the popal, so nothing but the flags is
// observable: they are those of the last compare executed.
// FUNCTION: GP4 0x005212c8
void __cdecl FUN_005212c8(gp4::Regs* r) {
    uint32_t arith = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007c2e82), 0xffu);
    if (GP4_GLOBAL(uint8_t, 0x007c2e82) != 0xffu) {
        arith = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007c2ea9), 0x80u);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005212c8, FUN_005212c8, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
