#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005be2e0 { uint8_t Unknown00[0x1b4]; float Value1b4; };
#pragma pack(pop)

// Returns the float at +0x1b4 of the object (fld dword [ecx+0x1b4]).
// FUNCTION: GP4 0x005be2e0
float __fastcall FUN_005be2e0(Unknown_005be2e0* self, void* /*edx*/) { return self->Value1b4; }
GP4_IMPL(0x005be2e0, FUN_005be2e0, "thiscall(ptr[0x1c0]:f32) -> f32")
