#include <gp4/gp4.h>
#include <gp4/flags.h>

// Colour-conversion self test (GP3 heritage): for ax = 0, 9, 18.. < 0x100 and cx = 0..0xff (bx stays 0: the
// bl loop's `cmp cx, 0x100` tests cx, which is already 0x100, so it never repeats) the three bytes
// al, bl, cl are stored at 0x00933a0e..10, the (empty) loader 0x0051b03f, RGB -> HSV 0x0051b1f9, HSV -> RGB
// 0x0051b2ea and the (empty) reader 0x0051b040 run, and the bytes are compared with the stored ones - on a
// mismatch the (empty) report routine 0x0051bf6a runs. The empty routine 0x0051bf6b ends the test.
// pushal/popal keep every register; the flags are those of the last `cmp ax, 0x100`.
// FUNCTION: GP4 0x0051bef2
void __cdecl FUN_0051bef2(gp4::Regs* r) {
    gp4::Regs c = *r;
    int16_t ax = 0;
    do {
        int16_t bx = 0;
        int16_t cx;
        do {
            cx = 0;
            do {
                GP4_GLOBAL(uint8_t, 0x00933a0e) = static_cast<uint8_t>(ax);
                GP4_GLOBAL(uint8_t, 0x00933a0f) = static_cast<uint8_t>(bx);
                GP4_GLOBAL(uint8_t, 0x00933a10) = static_cast<uint8_t>(cx);
                gp4::cdecl_<0x0051b03f, void>();
                gp4::call_regs(0x0051b1f9, c);
                gp4::call_regs(0x0051b2ea, c);
                gp4::cdecl_<0x0051b040, void>();
                if (static_cast<uint8_t>(ax) != GP4_GLOBAL(uint8_t, 0x00933a0e) ||
                    static_cast<uint8_t>(bx) != GP4_GLOBAL(uint8_t, 0x00933a0f) ||
                    static_cast<uint8_t>(cx) != GP4_GLOBAL(uint8_t, 0x00933a10)) {
                    gp4::cdecl_<0x0051bf6a, void>();
                }
                cx = static_cast<int16_t>(cx + 1);
            } while (cx < 0x100);
            bx = static_cast<int16_t>(bx + 5);
        } while (cx < 0x100);
        ax = static_cast<int16_t>(ax + 9);
    } while (ax < 0x100);
    gp4::cdecl_<0x0051bf6b, void>();

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(static_cast<uint16_t>(ax), 0x100u));
}
GP4_IMPL(0x0051bef2, FUN_0051bef2, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rgb_0051b1f9; maxinsn=20000000; wall=3600")
