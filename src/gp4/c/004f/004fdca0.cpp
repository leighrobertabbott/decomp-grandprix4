#include <gp4/gp4.h>

// Narrows the zero-terminated 16-bit string `src` into the byte buffer `dst`, dropping the characters
// below 0x20 and keeping the low byte of the others, for at most `count` source characters. The result
// is terminated: with a 0 after the last copied character when the source ended first, else by
// overwriting the last written byte (dst[-1]) when `count` ran out.
// FUNCTION: GP4 0x004fdca0
void __stdcall FUN_004fdca0(const uint16_t* src, char* dst, int count) {
    uint16_t c = *src;
    while (c != 0) {
        if (count == 0) {
            dst[-1] = '\0';
            return;
        }
        if (c >= 0x20) {
            *dst = static_cast<char>(*src);
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
GP4_IMPL(0x004fdca0, FUN_004fdca0, "stdcall(ptr[0x80]:u16{=0,0x0a,0x01,0x1f,0x20,0x41,0x61,0x7f,0x80,0xff,0x141,0xff00}, ptr[0x40]:bytes, i32[1..0x30]) -> void")
