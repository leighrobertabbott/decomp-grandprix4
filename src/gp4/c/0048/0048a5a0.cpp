#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0048a5a0 { uint8_t Unknown00[0xcc]; int32_t Value0cc; };
#pragma pack(pop)

// Stores its stack argument at +0xcc of the object.
// FUNCTION: GP4 0x0048a5a0
void __fastcall FUN_0048a5a0(Unknown_0048a5a0* self, void* /*edx*/, int32_t value) { self->Value0cc = value; }
GP4_IMPL(0x0048a5a0, FUN_0048a5a0, "thiscall(ptr[0xd8]:bytes, i32) -> void")
