#include <gp4/gp4.h>
#include <gp4/flags.h>

// Range test on the words 0x00891210 (0x1000 <= v < 0x3000) and 0x0089120e (0 <= v <= 0x2000);
// both outcomes load al with 8, so the two byte tables indexed by edi receive 8 either way.
// The flags left behind are those of the last compare executed.
// FUNCTION: GP4 0x0050b651
void __cdecl FUN_0050b651(gp4::Regs* r) {
    const uint32_t index = r->edi;
    const int16_t a = GP4_GLOBAL(int16_t, 0x00891210);
    const int16_t b = GP4_GLOBAL(int16_t, 0x0089120e);

    uint32_t f = gp4::flags::sub16((uint16_t)a, 0x1000u);
    if (a >= 0x1000) {
        f = gp4::flags::sub16((uint16_t)a, 0x3000u);
        if (a < 0x3000) {
            f = gp4::flags::sub16((uint16_t)b, 0u);
            if (b >= 0) {
                f = gp4::flags::sub16((uint16_t)b, 0x2000u);
            }
        }
    }
    const uint8_t al = 8;

    GP4_ARRAY(uint8_t, 0x00996768)[index] = al;
    GP4_ARRAY(uint8_t, 0x009a6768)[index] = al;
    r->eax = (r->eax & 0xffffff00u) | al;
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0050b651, FUN_0050b651, "regs(eax:u32, edi:u32[0..0xff]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
