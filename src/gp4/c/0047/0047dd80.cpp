#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0047dd80 { uint8_t Unknown00[0xb0]; uint8_t Value0b0; };
#pragma pack(pop)

// Stores the low byte of its stack argument at +0xb0 of the object.
// FUNCTION: GP4 0x0047dd80
void __fastcall FUN_0047dd80(Unknown_0047dd80* self, void* /*edx*/, uint8_t value) { self->Value0b0 = value; }
GP4_IMPL(0x0047dd80, FUN_0047dd80, "thiscall(ptr[0xc0]:bytes, i32) -> void")
