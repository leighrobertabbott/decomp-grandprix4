#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cb4c0 { uint8_t Unknown00[0x24]; int32_t Value24; };
#pragma pack(pop)

// Returns the dword at +0x24 of the object.
// FUNCTION: GP4 0x005cb4c0
int32_t __fastcall FUN_005cb4c0(Unknown_005cb4c0* self, void* /*edx*/) { return self->Value24; }
GP4_IMPL(0x005cb4c0, FUN_005cb4c0, "thiscall(ptr[0x30]:bytes) -> i32")
