#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00462b00 { uint8_t Unknown00[4]; uint32_t Field04; uint8_t Unknown08[0x30]; uint32_t Field38; };
#pragma pack(pop)

// Clears the dwords at +4 and +0x38 of the object.
// FUNCTION: GP4 0x00462b00
void __fastcall FUN_00462b00(Unknown_00462b00* self, void* /*edx*/) {
    self->Field38 = 0;
    self->Field04 = 0;
}
GP4_IMPL(0x00462b00, FUN_00462b00, "thiscall(ptr[0x40]:bytes) -> void")
