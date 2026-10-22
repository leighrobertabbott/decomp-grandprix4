#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b17a0 { uint8_t Unknown00[4]; int32_t Value04; };
#pragma pack(pop)

// Returns the dword at +0x04 of the object.
// FUNCTION: GP4 0x005b17a0
int32_t __fastcall FUN_005b17a0(Unknown_005b17a0* self, void* /*edx*/) { return self->Value04; }
GP4_IMPL(0x005b17a0, FUN_005b17a0, "thiscall(ptr[0x10]:bytes) -> i32")
