#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00464140 { uint8_t Unknown00[0x68]; int32_t Value68; int32_t Value6c; };
#pragma pack(pop)

// Stores its two stack arguments at +0x68 and +0x6c of the object.
// FUNCTION: GP4 0x00464140
void __fastcall FUN_00464140(Unknown_00464140* self, void* /*edx*/, int32_t a, int32_t b) {
    self->Value68 = a;
    self->Value6c = b;
}
GP4_IMPL(0x00464140, FUN_00464140, "thiscall(ptr[0x78]:bytes, i32, i32) -> void")
