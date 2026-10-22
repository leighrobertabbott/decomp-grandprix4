#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cc420 { uint8_t Unknown00[0x2148c]; int32_t Value2148c; };
#pragma pack(pop)

// Returns the dword at +0x2148c of the object.
// FUNCTION: GP4 0x005cc420
int32_t __fastcall FUN_005cc420(Unknown_005cc420* self, void* /*edx*/) { return self->Value2148c; }
GP4_IMPL(0x005cc420, FUN_005cc420, "thiscall(ptr[0x21498]:bytes) -> i32")
