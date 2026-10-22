#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ab340 { uint8_t Unknown00[0x73]; uint8_t Byte73; };
#pragma pack(pop)

// Stores the low byte of its stack argument at +0x73 of the object and returns 0.
// FUNCTION: GP4 0x005ab340
int __fastcall FUN_005ab340(Unknown_005ab340* self, void* /*edx*/, uint8_t value) { self->Byte73 = value; return 0; }
GP4_IMPL(0x005ab340, FUN_005ab340, "thiscall(ptr[0x80]:bytes, i32) -> i32")
