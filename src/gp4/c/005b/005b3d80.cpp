#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b3d80 { uint8_t Unknown00[0x194]; uint8_t Flag194; };
#pragma pack(pop)

// Sets the byte at +0x194 of the object to 1 and returns 0.
// FUNCTION: GP4 0x005b3d80
int __fastcall FUN_005b3d80(Unknown_005b3d80* self, void* /*edx*/) { self->Flag194 = 1; return 0; }
GP4_IMPL(0x005b3d80, FUN_005b3d80, "thiscall(ptr[0x1a0]:bytes) -> i32")
