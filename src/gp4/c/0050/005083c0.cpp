#include <gp4/gp4.h>

typedef int(__stdcall* TextOutA_t)(void* hdc, int x, int y, const char* text, int count);

// Draws the control-character-terminated string `text` (bytes below 0x20 end it) at (x, y) on the
// device context stored in 0x00b1bfa4 with GDI TextOutA; returns TextOutA's result.
// FUNCTION: GP4 0x005083c0
int __stdcall FUN_005083c0(int x, int y, const uint8_t* text) {
    const uint8_t* end = text;
    while (*end >= 0x20u) {
        end++;
    }
    int count = static_cast<int>(end - text);
    return GP4_IAT(TextOutA_t, 0x005f0054)(GP4_GLOBAL(void*, 0x00b1bfa4), x, y,
                                           reinterpret_cast<const char*>(text), count);
}
GP4_IMPL(0x005083c0, FUN_005083c0, "stdcall(i32, i32, ptr[0x100]:bytes) -> i32; globals=fuzz:i32; emu=skip:win32 TextOutA")
