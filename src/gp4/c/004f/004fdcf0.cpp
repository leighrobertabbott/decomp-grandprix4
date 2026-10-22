#include <gp4/gp4.h>

// Copies the string `src` into `dst`, dropping the control characters 0x00..0x1f except '\n' (bytes with the
// top bit set are kept: the char compare is signed), for at most `count` source characters. The result is
// terminated: with a 0 after the last copied character when the source ended first, else by overwriting the
// last written byte (dst[-1]) when `count` ran out.
// FUNCTION: GP4 0x004fdcf0
void __stdcall FUN_004fdcf0(const char* src, char* dst, int count) {
    char c = *src;
    while (c != '\0') {
        if (count == 0) {
            dst[-1] = '\0';
            return;
        }
        if (c < 0 || c > 0x1f || c == '\n') {
            *dst = c;
            dst++;
        }
        src++;
        count--;
        c = *src;
    }
    if (count != 0) {
        *dst = '\0';
        return;
    }
    dst[-1] = '\0';
}
GP4_IMPL(0x004fdcf0, FUN_004fdcf0, "stdcall(ptr[0x40]:u8{=0,0x0a,0x01,0x1f,0x20,0x41,0x61,0x7f,0x80,0xff}, ptr[0x40]:bytes, i32[1..0x30]) -> void")
