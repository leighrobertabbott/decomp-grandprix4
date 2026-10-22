#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b3db0 { uint8_t Unknown00[0x195]; uint8_t Flag195; };
#pragma pack(pop)

// Sets the byte at +0x195 of the object to 1 and returns 0.
// FUNCTION: GP4 0x005b3db0
int __fastcall FUN_005b3db0(Unknown_005b3db0* self, void* /*edx*/) { self->Flag195 = 1; return 0; }
GP4_IMPL(0x005b3db0, FUN_005b3db0, "thiscall(ptr[0x1a0]:bytes) -> i32")
