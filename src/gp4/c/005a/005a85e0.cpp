#include <gp4/gp4.h>

struct Unknown_005a85e0 { uint8_t Unknown00[0x3c]; uint8_t Flag3c; };

// Sets the byte at +0x3c of the object to 1.
// FUNCTION: GP4 0x005a85e0
void __fastcall FUN_005a85e0(Unknown_005a85e0* self, void* /*edx*/) { self->Flag3c = 1; }
GP4_IMPL(0x005a85e0, FUN_005a85e0, "thiscall(ptr[0x48]:bytes) -> void")
