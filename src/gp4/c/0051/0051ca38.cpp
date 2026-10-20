#include <gp4/gp4.h>
#include <gp4/flags.h>

// Splits the word at 0x00934dd8 (biased by 0x4000, sign-extended, arithmetic >> 4, masked to 12
// bits) into a high nibble-pair part 0x00811a50 (bits 8..11) and a low byte 0x00811a54 (bits 0..7).
// When the byte flag 0x007ad869 is non-zero the two are copied to 0x00a69880 (low) and
// 0x00a69884 (high). pushal/popal wrapper: every GPR is restored; the flags are those of
// `cmp byte [0x7ad869], 0` (popal leaves them).
// FUNCTION: GP4 0x0051ca38
void __cdecl FUN_0051ca38(gp4::Regs* r) {
    const uint16_t raw = GP4_GLOBAL(uint16_t, 0x00934dd8);
    const int32_t biased = static_cast<int16_t>(static_cast<uint16_t>(raw - 0x4000u));
    const uint32_t value = static_cast<uint32_t>(biased >> 4) & 0xfffu;
    GP4_GLOBAL(uint32_t, 0x00811a50) = value >> 8;
    GP4_GLOBAL(uint32_t, 0x00811a54) = value & 0xffu;
    const uint8_t flag = GP4_GLOBAL(uint8_t, 0x007ad869);
    if (flag != 0) {
        GP4_GLOBAL(uint32_t, 0x00a69880) = GP4_GLOBAL(uint32_t, 0x00811a54);
        GP4_GLOBAL(uint32_t, 0x00a69884) = GP4_GLOBAL(uint32_t, 0x00811a50);
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(flag, 0u));
}
GP4_IMPL(0x0051ca38, FUN_0051ca38, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
