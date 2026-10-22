#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0050a0a0 { uint32_t Field00; uint32_t Field04; uint32_t Field08; uint32_t Field0c; uint16_t Field10; uint8_t Unknown12[0xee]; };
#pragma pack(pop)

// Initialiser: clears the four dwords at +0..+0xc and the word at +0x10.
// FUNCTION: GP4 0x0050a0a0
void __fastcall FUN_0050a0a0(Unknown_0050a0a0* self, void* /*edx*/) {
    self->Field00 = 0;
    self->Field04 = 0;
    self->Field08 = 0;
    self->Field0c = 0;
    self->Field10 = 0;
}
GP4_IMPL(0x0050a0a0, FUN_0050a0a0, "thiscall(ptr[0x100]:bytes) -> void")
