#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046f590 { uint8_t Unknown00[0xc7c]; uint8_t Value0c7c; uint8_t Unknown0c7d[0x23]; };
#pragma pack(pop)

// Stores the byte argument at +0xc7c of the object.
// FUNCTION: GP4 0x0046f590
void __fastcall FUN_0046f590(Unknown_0046f590* self, void* /*edx*/, uint8_t value) { self->Value0c7c = value; }
GP4_IMPL(0x0046f590, FUN_0046f590, "thiscall(ptr[0xca0]:bytes, i32) -> void")
