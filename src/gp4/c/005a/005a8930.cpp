#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a8930 { uint8_t Unknown00[0x3d]; uint8_t Flag3d; };
#pragma pack(pop)

// Sets the byte at +0x3d of the object to 1 and returns 0.
// FUNCTION: GP4 0x005a8930
int32_t __fastcall FUN_005a8930(Unknown_005a8930* self, void* /*edx*/) { self->Flag3d = 1; return 0; }
GP4_IMPL(0x005a8930, FUN_005a8930, "thiscall(ptr[0x48]:bytes) -> i32")
