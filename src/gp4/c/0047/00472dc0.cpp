#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00472dc0 { uint8_t Unknown00[0x484]; int32_t Value484; uint8_t Unknown488[0x78]; };
#pragma pack(pop)

// Stores the dword argument at +0x484 of the object.
// FUNCTION: GP4 0x00472dc0
void __fastcall FUN_00472dc0(Unknown_00472dc0* self, void* /*edx*/, int value) { self->Value484 = value; }
GP4_IMPL(0x00472dc0, FUN_00472dc0, "thiscall(ptr[0x500]:bytes, i32) -> void")
