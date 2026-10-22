#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_004ea590 { uint8_t Unknown00[0x44]; int32_t Value44; };
#pragma pack(pop)

// Setter: stores its stack argument at +0x44.
// FUNCTION: GP4 0x004ea590
void __fastcall FUN_004ea590(Object_004ea590* self, void* /*edx*/, int32_t value) {
    self->Value44 = value;
}
GP4_IMPL(0x004ea590, FUN_004ea590, "thiscall(ptr[0x60]:bytes, i32) -> void")
