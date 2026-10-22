#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0047dd60 { uint8_t Unknown00[0xb1]; uint8_t Flag0b1; uint8_t Unknown0b2[2]; uint32_t Value0b4; uint8_t Unknown0b8[0x48]; };
#pragma pack(pop)

// Setter: sets the flag byte at +0xb1 and stores the stack argument in the dword at +0xb4.
// FUNCTION: GP4 0x0047dd60
void __fastcall FUN_0047dd60(Unknown_0047dd60* self, void* /*edx*/, uint32_t value) {
    self->Flag0b1 = 1;
    self->Value0b4 = value;
}
GP4_IMPL(0x0047dd60, FUN_0047dd60, "thiscall(ptr[0x100]:bytes, i32) -> void")
