#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00472da0 { uint8_t Unknown00[0x47d]; uint8_t Value047d; uint8_t Unknown047e[0x22]; };
#pragma pack(pop)

// Stores the byte argument at +0x47d of the object.
// FUNCTION: GP4 0x00472da0
void __fastcall FUN_00472da0(Unknown_00472da0* self, void* /*edx*/, uint8_t value) { self->Value047d = value; }
GP4_IMPL(0x00472da0, FUN_00472da0, "thiscall(ptr[0x4a0]:bytes, i32) -> void")
