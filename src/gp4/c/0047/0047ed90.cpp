#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0047ed90 { uint8_t Unknown00[8]; uint8_t Flag08; uint8_t Unknown09[0xf7]; };
#pragma pack(pop)

// Sets the byte flag at +8 of the object.
// FUNCTION: GP4 0x0047ed90
void __fastcall FUN_0047ed90(Unknown_0047ed90* self, void* /*edx*/) { self->Flag08 = 1; }
GP4_IMPL(0x0047ed90, FUN_0047ed90, "thiscall(ptr[0x100]:bytes) -> void")
