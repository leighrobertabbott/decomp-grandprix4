#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cb2e0 { uint8_t Unknown00[0x20]; float Value20; };
#pragma pack(pop)

// Returns the float at +0x20 of the object (fld dword [ecx+0x20]).
// FUNCTION: GP4 0x005cb2e0
float __fastcall FUN_005cb2e0(Unknown_005cb2e0* self, void* /*edx*/) { return self->Value20; }
GP4_IMPL(0x005cb2e0, FUN_005cb2e0, "thiscall(ptr[0x30]:f32) -> f32")
