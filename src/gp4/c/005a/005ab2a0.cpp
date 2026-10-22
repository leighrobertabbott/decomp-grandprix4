#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ab2a0 { uint8_t Unknown00[0x48]; int32_t Value48; };
#pragma pack(pop)

// Returns the dword at +0x48 of the object.
// FUNCTION: GP4 0x005ab2a0
int32_t __fastcall FUN_005ab2a0(Unknown_005ab2a0* self, void* /*edx*/) { return self->Value48; }
GP4_IMPL(0x005ab2a0, FUN_005ab2a0, "thiscall(ptr[0x50]:bytes) -> i32")
