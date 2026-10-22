#include <gp4/gp4.h>

// Forwards its stack argument to FUN_005da7d0 on the embedded sub-object at +0xe0.
// FUNCTION: GP4 0x005d4420
void __fastcall FUN_005d4420(void* self, void* /*edx*/, int32_t value) {
    gp4::thiscall_<0x005da7d0, void>((uint8_t*)self + 0xe0, value);
}
GP4_IMPL(0x005d4420, FUN_005d4420, "thiscall(ptr[0x140]:bytes, i32) -> void")
