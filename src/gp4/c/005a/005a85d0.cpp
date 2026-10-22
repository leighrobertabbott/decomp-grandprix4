#include <gp4/gp4.h>

struct Unknown_005a85d0 { uint8_t Unknown00[0x3c]; uint8_t Flag3c; };

// Clears the byte at +0x3c of the object.
// FUNCTION: GP4 0x005a85d0
void __fastcall FUN_005a85d0(Unknown_005a85d0* self, void* /*edx*/) { self->Flag3c = 0; }
GP4_IMPL(0x005a85d0, FUN_005a85d0, "thiscall(ptr[0x48]:bytes) -> void")
