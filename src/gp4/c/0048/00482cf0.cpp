#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00482cf0 { uint8_t Unknown00[0x404bc]; uint8_t Flag404bc; };
#pragma pack(pop)

// Sets the byte flag at +0x404bc of the object to 1.
// FUNCTION: GP4 0x00482cf0
void __fastcall FUN_00482cf0(Unknown_00482cf0* self, void* /*edx*/) { self->Flag404bc = 1; }
GP4_IMPL(0x00482cf0, FUN_00482cf0, "thiscall(ptr[0x40500]:bytes) -> void")
