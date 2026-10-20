#include <gp4/gp4.h>

// Sets the text colour of the device context [0x00b1bfa4] (thunk 0x005a497e: jmp [SetTextColor]),
// unless the byte switch [0x007ab53c] is set.
// FUNCTION: GP4 0x00546672
void __stdcall FUN_00546672(int32_t colour) {
    if (GP4_GLOBAL(uint8_t, 0x007ab53c) == 0u) {
        gp4::stdcall_<0x005a497e, int32_t>(GP4_GLOBAL(int32_t, 0x00b1bfa4), colour);
    }
}
GP4_IMPL(0x00546672, FUN_00546672, "stdcall(i32) -> void; globals=fuzz:i32")
