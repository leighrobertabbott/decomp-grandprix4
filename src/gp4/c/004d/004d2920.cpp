#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d2920 { uint8_t Unknown00[0x6c]; uint16_t Value6c; };
#pragma pack(pop)

// Stores the 16-bit argument at +0x6c of the object.
// FUNCTION: GP4 0x004d2920
void __fastcall FUN_004d2920(Unknown_004d2920* self, void* /*edx*/, uint16_t value) {
    self->Value6c = value;
}
GP4_IMPL(0x004d2920, FUN_004d2920, "thiscall(ptr[0x80]:bytes, u16) -> void")
