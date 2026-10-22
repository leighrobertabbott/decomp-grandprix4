#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a8300 { uint8_t Unknown00[0xf4]; int32_t Valuef4; };
#pragma pack(pop)

// Returns the dword at +0xf4 of the object.
// FUNCTION: GP4 0x005a8300
int32_t __fastcall FUN_005a8300(Unknown_005a8300* self, void* /*edx*/) { return self->Valuef4; }
GP4_IMPL(0x005a8300, FUN_005a8300, "thiscall(ptr[0x100]:bytes) -> i32")
