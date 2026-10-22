#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005da7d0 { uint8_t Unknown00[0x38]; int32_t Value38; uint8_t Unknown3c[4]; int32_t Value40; };
#pragma pack(pop)

// Sets the dword at +0x38 to 3 and stores its stack argument into the dword at +0x40.
// FUNCTION: GP4 0x005da7d0
void __fastcall FUN_005da7d0(Unknown_005da7d0* self, void* /*edx*/, int32_t value) {
    self->Value38 = 3;
    self->Value40 = value;
}
GP4_IMPL(0x005da7d0, FUN_005da7d0, "thiscall(ptr[0x48]:bytes, i32) -> void")
