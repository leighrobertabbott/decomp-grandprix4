#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045e4b0 {
    uint8_t Unknown00[0x18];
    uint32_t Value18;
    uint32_t Count1c;
    uint32_t Stack20[8];
};
#pragma pack(pop)

// Pushes the dword at +0x18 onto the 8-slot stack at +0x20 (count at +0x1c); returns true when it
// fit, false when the stack was already full.
// FUNCTION: GP4 0x0045e4b0
uint8_t __fastcall FUN_0045e4b0(Unknown_0045e4b0* self, void* /*edx*/) {
    uint32_t count = self->Count1c;
    if (count < 8) {
        self->Stack20[count] = self->Value18;
        self->Count1c = self->Count1c + 1;
        return 1;
    }
    return 0;
}
GP4_IMPL(0x0045e4b0, FUN_0045e4b0, "thiscall(ptr[0x48]:{0x18:u32, 0x1c:u32[0..10]}) -> u8")
