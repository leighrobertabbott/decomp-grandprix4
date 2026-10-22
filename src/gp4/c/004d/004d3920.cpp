#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d3920 { uint8_t Unknown00[0x68]; uint16_t Value68; };
#pragma pack(pop)

// Stores the 16-bit argument at +0x68 of the object.
// FUNCTION: GP4 0x004d3920
void __fastcall FUN_004d3920(Unknown_004d3920* self, void* /*edx*/, uint16_t value) {
    self->Value68 = value;
}
GP4_IMPL(0x004d3920, FUN_004d3920, "thiscall(ptr[0x78]:bytes, u16) -> void")
