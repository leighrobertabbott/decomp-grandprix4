#include <gp4/gp4.h>

// Copies the bytes >= 0x14 of a text string (up to the first control byte or NUL, which is not
// copied) into the text/frame buffer, centred: destination = base pointer 0x831568 + x + y * row
// pitch 0xb1bf40 - (length / 2), the length coming from 0x00546249. eax, esi and edi are saved and
// restored; the flags are never consumed (callers return straight to a caller that ignores them).
// FUNCTION: GP4 0x0054657d
void __stdcall FUN_0054657d(int x, int y, const uint8_t* text) {
    gp4::Regs measure = {};
    measure.esi = (uint32_t)(uintptr_t)text;
    gp4::call_regs(0x00546249, measure);
    const uint32_t half = measure.eax >> 1;

    uint32_t address = (uint32_t)y * GP4_GLOBAL(uint32_t, 0x00b1bf40);
    address += (uint32_t)x;
    address += GP4_GLOBAL(uint32_t, 0x00831568);
    address -= half;

    uint8_t* dst = (uint8_t*)(uintptr_t)address;
    while (*text >= 0x14u) {
        *dst++ = *text++;
    }
}
GP4_IMPL(0x0054657d, FUN_0054657d, "stdcall(i32[0..64], i32[0..8], ptr[0x200]:bytes) -> void; globals=fuzz:i32+layout=surf_0054654a")
