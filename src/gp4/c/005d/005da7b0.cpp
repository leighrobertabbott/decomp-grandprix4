#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005da7b0 { uint8_t Unknown00[0x38]; int32_t Value38; uint8_t Unknown3c[4]; int32_t Value40; uint8_t Unknown44[0xbc]; };
#pragma pack(pop)

// Sets the dword at +0x38 of the object to 2 and stores the stack argument at +0x40 (`ret 4`).
// FUNCTION: GP4 0x005da7b0
void __fastcall FUN_005da7b0(Unknown_005da7b0* self, void* /*edx*/, int32_t value) {
    self->Value38 = 2;
    self->Value40 = value;
}
GP4_IMPL(0x005da7b0, FUN_005da7b0, "thiscall(ptr[0x100]:bytes, i32) -> void")
