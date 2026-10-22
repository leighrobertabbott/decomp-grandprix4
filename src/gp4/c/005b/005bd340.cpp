#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bd340 { uint8_t Unknown00[0xbc]; int32_t Valuebc; };
#pragma pack(pop)

// Stores its stack argument into the dword at +0xbc of the object.
// FUNCTION: GP4 0x005bd340
void __fastcall FUN_005bd340(Unknown_005bd340* self, void* /*edx*/, int32_t value) { self->Valuebc = value; }
GP4_IMPL(0x005bd340, FUN_005bd340, "thiscall(ptr[0xc8]:bytes, i32) -> void")
