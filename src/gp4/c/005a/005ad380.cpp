#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ad380 { uint8_t Unknown00[0x1a4]; int32_t Value1a4; };
#pragma pack(pop)

// Returns the dword at +0x1a4 of the object.
// FUNCTION: GP4 0x005ad380
int32_t __fastcall FUN_005ad380(Unknown_005ad380* self, void* /*edx*/) { return self->Value1a4; }
GP4_IMPL(0x005ad380, FUN_005ad380, "thiscall(ptr[0x1b0]:bytes) -> i32")
