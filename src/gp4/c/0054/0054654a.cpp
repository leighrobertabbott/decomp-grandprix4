#include <gp4/gp4.h>

// FUNCTION: GP4 0x0054654a
// Copies the bytes of a text string (everything >= 0x14, i.e. up to the first control byte or
// NUL, which is not copied) into a text/frame buffer: destination = base pointer 0x831568 + x +
// y * row pitch 0xb1bf40. eax, esi and edi are saved and restored; the flags are never consumed
// (the callers' next instruction is a call).
void __stdcall FUN_0054654a(int x, int y, const uint8_t* text) {
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(
        static_cast<uint32_t>(y * GP4_GLOBAL(int, 0x00b1bf40) + x + GP4_GLOBAL(int, 0x00831568))));
    while (*text >= 0x14u) {
        *dst++ = *text++;
    }
}
GP4_IMPL(0x0054654a, FUN_0054654a, "stdcall(i32[0..64], i32[0..8], ptr[0x200]:bytes) -> void; globals=fuzz:i32+layout=surf_0054654a")
