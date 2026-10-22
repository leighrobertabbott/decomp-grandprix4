#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d2930 {
    uint8_t Unknown00[0x74];
    uint16_t Value74;
    uint16_t Value76;
};
#pragma pack(pop)

// Stores the 16-bit value at +0x74 when the selector byte is 1, otherwise at +0x76.
// FUNCTION: GP4 0x004d2930
void __fastcall FUN_004d2930(Unknown_004d2930* self, void* /*edx*/, uint8_t which, uint16_t value) {
    if (which == 1) {
        self->Value74 = value;
        return;
    }
    self->Value76 = value;
}
GP4_IMPL(0x004d2930, FUN_004d2930, "thiscall(ptr[0x80]:bytes, u8{0,1,2,0xff}, u16) -> void")
