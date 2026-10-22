#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00449050 { uint8_t Unknown00[0x60]; int32_t Value60; uint8_t Unknown64[0x9c]; };
#pragma pack(pop)

// Stores -1 in the dword at +0x60 of the object.
// FUNCTION: GP4 0x00449050
void __fastcall FUN_00449050(Unknown_00449050* self, void* /*edx*/) { self->Value60 = -1; }
GP4_IMPL(0x00449050, FUN_00449050, "thiscall(ptr[0x100]:bytes) -> void")
