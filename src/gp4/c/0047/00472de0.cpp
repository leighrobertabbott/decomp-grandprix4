#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00472de0 { uint8_t Unknown00[0x488]; uint8_t Flag488; uint8_t Unknown489[0x77]; };
#pragma pack(pop)

// Stores the low byte of the argument at +0x488 of the object.
// FUNCTION: GP4 0x00472de0
void __fastcall FUN_00472de0(Unknown_00472de0* self, void* /*edx*/, int value) { self->Flag488 = (uint8_t)value; }
GP4_IMPL(0x00472de0, FUN_00472de0, "thiscall(ptr[0x500]:bytes, i32) -> void")
