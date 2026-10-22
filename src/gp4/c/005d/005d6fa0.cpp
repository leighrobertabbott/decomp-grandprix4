#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005d6fa0 { uint8_t Unknown00[0x28]; uint32_t Value28; uint8_t Unknown2c[0x1e98]; uint8_t Flag1ec4; };
#pragma pack(pop)

// Copies the dword at +0x28 of the object into the global at 0x00b4764c, sets the byte at +0x1ec4
// to 1 and returns that dword with its low byte cleared (`xor al, al`).
// FUNCTION: GP4 0x005d6fa0
uint32_t __fastcall FUN_005d6fa0(Unknown_005d6fa0* self, void* /*edx*/) {
    uint32_t value = self->Value28;
    GP4_GLOBAL(uint32_t, 0x00b4764c) = value;
    self->Flag1ec4 = 1;
    return value & 0xffffff00u;
}
GP4_IMPL(0x005d6fa0, FUN_005d6fa0, "thiscall(ptr[0x1ed0]:bytes) -> u32; globals=fuzz:i32")
