#include <gp4/gp4.h>

// Selects a GDI object into the device context [0x00b1bfa4] (thunk 0x005a4984: jmp [SelectObject]),
// unless the byte switch [0x007ab53c] is set.
// FUNCTION: GP4 0x00546654
void __stdcall FUN_00546654(int32_t object) {
    if (GP4_GLOBAL(uint8_t, 0x007ab53c) == 0u) {
        gp4::stdcall_<0x005a4984, int32_t>(GP4_GLOBAL(int32_t, 0x00b1bfa4), object);
    }
}
GP4_IMPL(0x00546654, FUN_00546654, "stdcall(i32) -> void; globals=fuzz:i32")
