#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00472db0 { uint8_t Unknown00[0x489]; uint8_t Flag489; uint8_t Unknown48a[0x76]; };
#pragma pack(pop)

// Stores the low byte of the argument at +0x489 of the object.
// FUNCTION: GP4 0x00472db0
void __fastcall FUN_00472db0(Unknown_00472db0* self, void* /*edx*/, int value) { self->Flag489 = (uint8_t)value; }
GP4_IMPL(0x00472db0, FUN_00472db0, "thiscall(ptr[0x500]:bytes, i32) -> void")
