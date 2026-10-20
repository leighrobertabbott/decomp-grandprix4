#include <gp4/gp4.h>
#include <gp4/flags.h>

struct Unknown_007c30a3 { uint8_t Unknown00[0xa41]; };

// One-shot copy: when the flag byte 0x0085306c is zero, the 0xa41 bytes 0x007c30a3..0x007c3ae4 are
// copied to 0x0085306d and the flag byte is set to 0xff. pushal/popal wrapper: every GPR is restored;
// the flags are those of "cmp byte [0x85306c],0" when the copy is already done, otherwise those of
// "cmp ecx,0xa41" (ecx = 0x7c3ae4 - 0x7c30a3, so ZF and PF set).
// FUNCTION: GP4 0x00520cf4
void __cdecl FUN_00520cf4(gp4::Regs* r) {
    uint32_t arith = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x0085306c), 0u);
    if (GP4_GLOBAL(uint8_t, 0x0085306c) == 0u) {
        arith = gp4::flags::sub32(0x7c3ae4u - 0x7c30a3u, 0xa41u);
        GP4_GLOBAL(Unknown_007c30a3, 0x0085306d) = GP4_GLOBAL(Unknown_007c30a3, 0x007c30a3);
        GP4_GLOBAL(uint8_t, 0x0085306c) = 0xffu;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00520cf4, FUN_00520cf4, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
