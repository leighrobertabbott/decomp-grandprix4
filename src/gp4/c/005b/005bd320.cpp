#include <gp4/gp4.h>

// Forwards its two stack arguments to 0x005c8c40 on the same object (ecx is passed through
// untouched; the first argument is pushed last, i.e. stays first).
// FUNCTION: GP4 0x005bd320
void __fastcall FUN_005bd320(void* self, void* /*edx*/, int32_t first, int32_t second) {
    gp4::thiscall_<0x005c8c40, void>(self, first, second);
}
GP4_IMPL(0x005bd320, FUN_005bd320, "thiscall(ptr[0x100]:bytes, i32, i32) -> void")
