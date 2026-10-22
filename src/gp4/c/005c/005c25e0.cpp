#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005c25e0 { uint8_t Unknown00[0x48]; uint8_t Flag48; uint8_t Unknown49[0x20]; };
#pragma pack(pop)

// Sets the byte flag at +0x48 of the object: `mov byte ptr [ecx + 0x48], 1; ret`.
// FUNCTION: GP4 0x005c25e0
void __fastcall FUN_005c25e0(Unknown_005c25e0* self, void* /*edx*/) { self->Flag48 = 1; }
GP4_IMPL(0x005c25e0, FUN_005c25e0, "thiscall(ptr[0x100]:bytes) -> void")
