#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0048a5b0 { uint8_t Unknown00[0xd0]; int32_t Value0d0; };
#pragma pack(pop)

// Stores its stack argument at +0xd0 of the object.
// FUNCTION: GP4 0x0048a5b0
void __fastcall FUN_0048a5b0(Unknown_0048a5b0* self, void* /*edx*/, int32_t value) { self->Value0d0 = value; }
GP4_IMPL(0x0048a5b0, FUN_0048a5b0, "thiscall(ptr[0xd8]:bytes, i32) -> void")
