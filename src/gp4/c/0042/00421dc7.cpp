#include <gp4/gp4.h>

// (value * DAT_0060384c) >> 16 (low 32 bits of the 64-bit product shifted), clamped in magnitude to
// the unsigned limit DAT_00607ac4, then shifted left by 8.
static inline uint32_t scaledClamp(int32_t value) {
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060384c));
    uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 16);
    if (static_cast<int32_t>(scaled) < 0) {
        uint32_t magnitude = 0u - scaled;
        if (!(magnitude < GP4_GLOBAL(uint32_t, 0x00607ac4))) {
            magnitude = GP4_GLOBAL(uint32_t, 0x00607ac4);
        }
        scaled = 0u - magnitude;
    } else if (!(scaled < GP4_GLOBAL(uint32_t, 0x00607ac4))) {
        scaled = GP4_GLOBAL(uint32_t, 0x00607ac4);
    }
    return scaled << 8;
}

// While DAT_00609840 is set: the three scaled and clamped vector components DAT_00607b00/04/08
// (<< 8) go to DAT_00629dd4/d8/dc, the angle of the vector (DAT_00609844, DAT_0060984c) from
// 0x00401315 selects the first of the 20 ascending bounds at 0x00629f44 that it is below (0 when
// none), and 0x004389a5 is run for the matching entry of 0x00629fe4 on the record in esi with
// DAT_00629f30 = 0xff and DAT_00629f40 temporarily DAT_00629f3c. pushal/popal keep every register;
// the flags are those of that call (DAT_00609840 compared with zero when it is clear).
// FUNCTION: GP4 0x00421dc7
void __cdecl FUN_00421dc7(gp4::Regs* r) {
    uint32_t flags = 0x44u;
    if (GP4_GLOBAL(uint32_t, 0x00609840) != 0) {
        GP4_GLOBAL(uint32_t, 0x00629dd4) = scaledClamp(GP4_GLOBAL(int32_t, 0x00607b00));
        GP4_GLOBAL(uint32_t, 0x00629dd8) = scaledClamp(GP4_GLOBAL(int32_t, 0x00607b04));
        GP4_GLOBAL(uint32_t, 0x00629ddc) = scaledClamp(GP4_GLOBAL(int32_t, 0x00607b08));

        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eax = GP4_GLOBAL(uint32_t, 0x00609844);
        c.edx = GP4_GLOBAL(uint32_t, 0x0060984c);
        gp4::call_regs(0x00401315, c);
        const uint16_t angle = static_cast<uint16_t>(c.eax);

        uint32_t index = 0;
        for (;;) {
            if (angle < GP4_ARRAY(uint16_t, 0x00629f44)[index]) {
                break;
            }
            ++index;
            if (index == 0x14u) {
                index = 0;
                break;
            }
        }

        GP4_GLOBAL(uint8_t, 0x00629f30) = 0xff;
        const uint32_t saved = GP4_GLOBAL(uint32_t, 0x00629f40);
        GP4_GLOBAL(uint32_t, 0x00629f40) = GP4_GLOBAL(uint32_t, 0x00629f3c);
        c.ebx = index;
        c.eax = GP4_ARRAY(uint32_t, 0x00629fe4)[index];
        gp4::call_regs(0x004389a5, c);
        GP4_GLOBAL(uint32_t, 0x00629f40) = saved;
        GP4_GLOBAL(uint8_t, 0x00629f30) = 0;
        flags = c.eflags & 0x8d5u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00421dc7, FUN_00421dc7, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
