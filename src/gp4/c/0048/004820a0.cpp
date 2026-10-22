#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004820a0 { uint8_t Unknown00[0x1c]; int32_t Value1c; };
#pragma pack(pop)

// Returns the dword at +0x1c of the object.
// FUNCTION: GP4 0x004820a0
int32_t __fastcall FUN_004820a0(Unknown_004820a0* self, void* /*edx*/) { return self->Value1c; }
GP4_IMPL(0x004820a0, FUN_004820a0, "thiscall(ptr[0x40]:bytes) -> i32")
