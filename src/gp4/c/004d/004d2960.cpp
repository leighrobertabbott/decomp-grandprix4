#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d2960 { uint8_t Unknown00[0x72]; uint16_t Value72; };
#pragma pack(pop)

// Stores the 16-bit argument at +0x72 of the object.
// FUNCTION: GP4 0x004d2960
void __fastcall FUN_004d2960(Unknown_004d2960* self, void* /*edx*/, uint16_t value) {
    self->Value72 = value;
}
GP4_IMPL(0x004d2960, FUN_004d2960, "thiscall(ptr[0x80]:bytes, u16) -> void")
