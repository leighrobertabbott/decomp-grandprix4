#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b5b80 { uint8_t Unknown00[0x4c]; int32_t Value4c; };
#pragma pack(pop)

// Stores its stack argument into the dword at +0x4c of the object.
// FUNCTION: GP4 0x005b5b80
void __fastcall FUN_005b5b80(Unknown_005b5b80* self, void* /*edx*/, int32_t value) { self->Value4c = value; }
GP4_IMPL(0x005b5b80, FUN_005b5b80, "thiscall(ptr[0x58]:bytes, i32) -> void")
