#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a9d70 { uint8_t Unknown00[0x70]; uint8_t Byte70; };
#pragma pack(pop)

// Stores the low byte of its stack argument at +0x70 of the object and returns 0.
// FUNCTION: GP4 0x005a9d70
int __fastcall FUN_005a9d70(Unknown_005a9d70* self, void* /*edx*/, uint8_t value) { self->Byte70 = value; return 0; }
GP4_IMPL(0x005a9d70, FUN_005a9d70, "thiscall(ptr[0x78]:bytes, i32) -> i32")
