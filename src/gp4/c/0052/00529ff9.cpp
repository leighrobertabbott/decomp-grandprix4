#include <gp4/gp4.h>
#include <gp4/flags.h>

// Computes the base of a two-row layout: the selector 0x7c2f28 is halved, or (when the mode byte
// 0x7c2e75 is nonzero) its low byte is replaced by the table byte at 0x7c2b24[selector]. The result
// is stored as 0x19b + v and 0x190 + v. eax/ebx are saved and restored by the original, so the only
// register effect is the flags of the last `add ebx, eax`.
// FUNCTION: GP4 0x00529ff9
void __cdecl FUN_00529ff9(gp4::Regs* r) {
    const uint32_t selector = GP4_GLOBAL(uint32_t, 0x007c2f28);
    uint32_t v;
    if (GP4_GLOBAL(uint8_t, 0x007c2e75) == 0) {
        v = selector >> 1;
    } else {
        v = (selector & 0xffffff00u) | GP4_ARRAY(uint8_t, 0x007c2b24)[selector];
    }
    GP4_GLOBAL(uint32_t, 0x0081d336) = 0x19bu + v;
    GP4_GLOBAL(uint32_t, 0x0081d33a) = 0x190u + v;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(0x190u, v));
}
GP4_IMPL(0x00529ff9, FUN_00529ff9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sel_007c2f28")
