#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_004cb0f0 { uint8_t Unknown00[8]; int32_t Value08; };
#pragma pack(pop)

// Setter: stores its stack argument at +8.
// FUNCTION: GP4 0x004cb0f0
void __fastcall FUN_004cb0f0(Object_004cb0f0* self, void* /*edx*/, int32_t value) {
    self->Value08 = value;
}
GP4_IMPL(0x004cb0f0, FUN_004cb0f0, "thiscall(ptr[0x20]:bytes, i32) -> void")
