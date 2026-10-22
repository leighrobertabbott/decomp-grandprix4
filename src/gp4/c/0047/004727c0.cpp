#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004727c0 { uint8_t Unknown00[0x480]; int32_t Value480; };
#pragma pack(pop)

// Stores its stack argument at +0x480 of the object, but only while the dword global at
// 0x009fa9e0 is nonzero.
// FUNCTION: GP4 0x004727c0
void __fastcall FUN_004727c0(Unknown_004727c0* self, void* /*edx*/, int32_t value) {
    if (GP4_GLOBAL(uint32_t, 0x009fa9e0) != 0) {
        self->Value480 = value;
    }
}
GP4_IMPL(0x004727c0, FUN_004727c0, "thiscall(ptr[0x490]:bytes, i32) -> void; globals=fuzz:i32")
