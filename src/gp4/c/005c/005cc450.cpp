#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cc450 { uint8_t Unknown00[0x21488]; float Value21488; };
#pragma pack(pop)

// Returns the float at +0x21488 of the object (fld dword [ecx+0x21488]).
// FUNCTION: GP4 0x005cc450
float __fastcall FUN_005cc450(Unknown_005cc450* self, void* /*edx*/) { return self->Value21488; }
GP4_IMPL(0x005cc450, FUN_005cc450, "thiscall(ptr[0x21490]:f32) -> f32")
