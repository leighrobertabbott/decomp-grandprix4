#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005aaf60 { uint8_t Unknown00[0x68]; float Value68; };
#pragma pack(pop)

// Returns the float at +0x68 of the object (fld dword [ecx+0x68]).
// FUNCTION: GP4 0x005aaf60
float __fastcall FUN_005aaf60(Unknown_005aaf60* self, void* /*edx*/) { return self->Value68; }
GP4_IMPL(0x005aaf60, FUN_005aaf60, "thiscall(ptr[0x70]:f32) -> f32")
