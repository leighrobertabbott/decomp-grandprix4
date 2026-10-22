#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0047dd90 { uint8_t Unknown00[0xb8]; float Value0b8; };
#pragma pack(pop)

// Subtracts the float constant at 0x005f0880 from the float at +0xb8 of the object.
// FUNCTION: GP4 0x0047dd90
void __fastcall FUN_0047dd90(Unknown_0047dd90* self, void* /*edx*/) {
    self->Value0b8 = self->Value0b8 - GP4_GLOBAL(float, 0x005f0880);
}
GP4_IMPL(0x0047dd90, FUN_0047dd90, "thiscall(ptr[0xc0]:f32) -> void")
