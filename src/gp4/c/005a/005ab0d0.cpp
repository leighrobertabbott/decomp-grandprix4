#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ab0d0 { uint8_t Unknown00[0x76]; uint8_t Byte76; };
#pragma pack(pop)

// Stores the low byte of its stack argument at +0x76 of the object and returns 0.
// FUNCTION: GP4 0x005ab0d0
int __fastcall FUN_005ab0d0(Unknown_005ab0d0* self, void* /*edx*/, uint8_t value) { self->Byte76 = value; return 0; }
GP4_IMPL(0x005ab0d0, FUN_005ab0d0, "thiscall(ptr[0x80]:bytes, i32) -> i32")
