#include <gp4/gp4.h>

typedef int(__stdcall* LineTo_t)(void* hdc, int x, int y);

// Draws a line to (x, y) on the device context in 0x00b1bfa4 when the flag byte at 0x007ab53c
// is zero.
// FUNCTION: GP4 0x00508610
void __stdcall FUN_00508610(int x, int y) {
    if (GP4_GLOBAL(uint8_t, 0x007ab53c) == 0) {
        GP4_IAT(LineTo_t, 0x005f0040)(GP4_GLOBAL(void*, 0x00b1bfa4), x, y);
    }
}
GP4_IMPL(0x00508610, FUN_00508610, "stdcall(i32, i32) -> void; globals=fuzz:i32; emu=skip:win32 LineTo")
