#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cc440 { uint8_t Unknown00[0x21484]; float Value21484; };
#pragma pack(pop)

// Returns the float at +0x21484 of the object (fld dword [ecx+0x21484]).
// FUNCTION: GP4 0x005cc440
float __fastcall FUN_005cc440(Unknown_005cc440* self, void* /*edx*/) { return self->Value21484; }
GP4_IMPL(0x005cc440, FUN_005cc440, "thiscall(ptr[0x21490]:f32) -> f32")
