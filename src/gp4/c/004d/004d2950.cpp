#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d2950 { uint8_t Unknown00[0x78]; uint16_t Value78; };
#pragma pack(pop)

// Stores the 16-bit argument at +0x78 of the object.
// FUNCTION: GP4 0x004d2950
void __fastcall FUN_004d2950(Unknown_004d2950* self, void* /*edx*/, uint16_t value) {
    self->Value78 = value;
}
GP4_IMPL(0x004d2950, FUN_004d2950, "thiscall(ptr[0x88]:bytes, u16) -> void")
