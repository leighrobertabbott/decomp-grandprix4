#include <gp4/gp4.h>

typedef int(__stdcall* DeleteObject_t)(void* object);

// When the flag byte at 0x007ab53c is zero, deletes the GDI object held in 0x00b1bfb0 and clears it.
// FUNCTION: GP4 0x00508580
void __cdecl FUN_00508580() {
    if (GP4_GLOBAL(uint8_t, 0x007ab53c) == 0) {
        GP4_IAT(DeleteObject_t, 0x005f00b0)(GP4_GLOBAL(void*, 0x00b1bfb0));
        GP4_GLOBAL(void*, 0x00b1bfb0) = 0;
    }
}
GP4_IMPL(0x00508580, FUN_00508580, "cdecl() -> void; globals=fuzz:i32; emu=skip:win32 DeleteObject")
