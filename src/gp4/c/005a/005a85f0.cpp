#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a85f0 { uint8_t Unknown00[0x3b]; uint8_t Flag3b; };
#pragma pack(pop)

// Clears the byte at +0x3b of the object and returns 1.
// FUNCTION: GP4 0x005a85f0
int __fastcall FUN_005a85f0(Unknown_005a85f0* self, void* /*edx*/) { self->Flag3b = 0; return 1; }
GP4_IMPL(0x005a85f0, FUN_005a85f0, "thiscall(ptr[0x48]:bytes) -> i32")
