#include <gp4/gp4.h>

typedef int(__stdcall* SelectClipRgn_t)(void* hdc, void* region);

// Removes the clipping region of the device context in 0x00b1bfa4 when the flag byte at
// 0x007ab53c is zero.
// FUNCTION: GP4 0x005085c0
void __cdecl FUN_005085c0() {
    if (GP4_GLOBAL(uint8_t, 0x007ab53c) == 0) {
        GP4_IAT(SelectClipRgn_t, 0x005f0048)(GP4_GLOBAL(void*, 0x00b1bfa4), nullptr);
    }
}
GP4_IMPL(0x005085c0, FUN_005085c0, "cdecl() -> void; globals=fuzz:i32; emu=skip:win32 SelectClipRgn")
