#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cc430 { uint8_t Unknown00[0x21480]; float Value21480; };
#pragma pack(pop)

// Returns the float at +0x21480 of the object (fld dword [ecx+0x21480]).
// FUNCTION: GP4 0x005cc430
float __fastcall FUN_005cc430(Unknown_005cc430* self, void* /*edx*/) { return self->Value21480; }
GP4_IMPL(0x005cc430, FUN_005cc430, "thiscall(ptr[0x21490]:f32) -> f32")
