#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00472dd0 { uint8_t Unknown00[0x47e]; uint8_t Flag47e; uint8_t Unknown47f[0x81]; };
#pragma pack(pop)

// Stores the low byte of the argument at +0x47e of the object.
// FUNCTION: GP4 0x00472dd0
void __fastcall FUN_00472dd0(Unknown_00472dd0* self, void* /*edx*/, int value) { self->Flag47e = (uint8_t)value; }
GP4_IMPL(0x00472dd0, FUN_00472dd0, "thiscall(ptr[0x500]:bytes, i32) -> void")
