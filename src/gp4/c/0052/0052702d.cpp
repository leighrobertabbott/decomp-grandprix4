#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: asks 0x00409a38 about the selected car record [0x00679d30] (CF set when its word +0xea
// equals 0x20) and stores one of six words into 0x00933a9a: 0x00933a9c when CF, else 0x00933aa0 if the
// byte 0x007c2ea9 is 0x80 and 0x00933aa4 otherwise; each is replaced by its neighbour (+2) when the
// word 0x00875e92 is negative. popal restores every register; the flags are those of
// `test word [0x875e92], 0xffff`.
// FUNCTION: GP4 0x0052702d
void __cdecl FUN_0052702d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x00409a38, c);

    uint16_t value;
    if ((c.eflags & gp4::flags::CF) != 0u) {
        value = GP4_GLOBAL(uint16_t, 0x00933a9c);
        if (static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00875e92)) < 0) {
            value = GP4_GLOBAL(uint16_t, 0x00933a9e);
        }
    } else if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
        value = GP4_GLOBAL(uint16_t, 0x00933aa0);
        if (static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00875e92)) < 0) {
            value = GP4_GLOBAL(uint16_t, 0x00933aa2);
        }
    } else {
        value = GP4_GLOBAL(uint16_t, 0x00933aa4);
        if (static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00875e92)) < 0) {
            value = GP4_GLOBAL(uint16_t, 0x00933aa6);
        }
    }
    GP4_GLOBAL(uint16_t, 0x00933a9a) = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic16(GP4_GLOBAL(uint16_t, 0x00875e92)));
}
GP4_IMPL(0x0052702d, FUN_0052702d, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00418f8f+carea_0042f6ee+carno_0066ff6c")
