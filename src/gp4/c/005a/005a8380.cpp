#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a8380 { uint8_t Unknown00[0x48]; int32_t Value48; };
#pragma pack(pop)

// Stores its stack argument into the dword at +0x48 of the object and returns 0.
// FUNCTION: GP4 0x005a8380
int __fastcall FUN_005a8380(Unknown_005a8380* self, void* /*edx*/, int32_t value) { self->Value48 = value; return 0; }
GP4_IMPL(0x005a8380, FUN_005a8380, "thiscall(ptr[0x58]:bytes, i32) -> i32")
