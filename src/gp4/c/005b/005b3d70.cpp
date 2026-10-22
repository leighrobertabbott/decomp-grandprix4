#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b3d70 { uint8_t Unknown00[0x194]; uint8_t Flag194; };
#pragma pack(pop)

// Clears the byte at +0x194 of the object and returns 0.
// FUNCTION: GP4 0x005b3d70
int __fastcall FUN_005b3d70(Unknown_005b3d70* self, void* /*edx*/) { self->Flag194 = 0; return 0; }
GP4_IMPL(0x005b3d70, FUN_005b3d70, "thiscall(ptr[0x1a0]:bytes) -> i32")
