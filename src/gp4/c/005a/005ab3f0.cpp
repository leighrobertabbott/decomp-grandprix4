#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ab3f0 { uint8_t Unknown00[0x50]; int32_t Value50; };
#pragma pack(pop)

// Stores its stack argument into the dword at +0x50 of the object.
// FUNCTION: GP4 0x005ab3f0
void __fastcall FUN_005ab3f0(Unknown_005ab3f0* self, void* /*edx*/, int32_t value) { self->Value50 = value; }
GP4_IMPL(0x005ab3f0, FUN_005ab3f0, "thiscall(ptr[0x58]:bytes, i32) -> void")
