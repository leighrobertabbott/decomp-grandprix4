#include <gp4/gp4.h>
#include <gp4/flags.h>

// Replicates the word at index 5 of the 16-bit table 0x009e2310 into entries 0..4, unless it is
// 0xffff (unused). The table has two banks 0x700 words apart; byte [0x007ad869] selects the second.
// pushal/popal wrapper: every GPR is restored; the flags are those of the last flag writer
// (popal does not restore them): "cmp ax,-1" when nothing is copied, else the final "cmp ecx,5".
// FUNCTION: GP4 0x0051cc96
void __cdecl FUN_0051cc96(gp4::Regs* r) {
    uint32_t bank = 0u;
    if (GP4_GLOBAL(uint8_t, 0x007ad869) != 0u) {
        bank += 0x700u;
    }
    const uint16_t value = GP4_ARRAY(uint16_t, 0x009e231a)[bank];
    uint32_t arith = gp4::flags::sub16(value, 0xffffu);
    if (value != 0xffffu) {
        uint32_t i = 0u;
        do {
            GP4_ARRAY(uint16_t, 0x009e2310)[i + bank] = value;
            ++i;
        } while ((int32_t)i < 5);
        arith = gp4::flags::sub32(i, 5u);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051cc96, FUN_0051cc96, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
