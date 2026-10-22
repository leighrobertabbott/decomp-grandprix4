#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00472d90 { uint8_t Unknown00[0x47c]; uint8_t Value047c; uint8_t Unknown047d[0x23]; };
#pragma pack(pop)

// Stores the byte argument at +0x47c of the object.
// FUNCTION: GP4 0x00472d90
void __fastcall FUN_00472d90(Unknown_00472d90* self, void* /*edx*/, uint8_t value) { self->Value047c = value; }
GP4_IMPL(0x00472d90, FUN_00472d90, "thiscall(ptr[0x4a0]:bytes, i32) -> void")
