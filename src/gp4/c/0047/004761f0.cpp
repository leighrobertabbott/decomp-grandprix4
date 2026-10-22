#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004761f0 { uint8_t Byte00; uint8_t Unknown01[3]; uint32_t Field04; uint32_t Field08; uint32_t Field0c; uint8_t Unknown10[0xf0]; };
#pragma pack(pop)

// Initialiser: the original stores an uninitialised stack byte at +0 (`push ecx` leaves the
// high byte of the this pointer at [esp+3], which it copies), then clears the dwords at
// +4, +8 and +0xc.
// FUNCTION: GP4 0x004761f0
void __fastcall FUN_004761f0(Unknown_004761f0* self, void* /*edx*/) {
    self->Byte00 = (uint8_t)((uint32_t)(uintptr_t)self >> 24);
    self->Field04 = 0;
    self->Field08 = 0;
    self->Field0c = 0;
}
GP4_IMPL(0x004761f0, FUN_004761f0, "thiscall(ptr[0x100]:bytes) -> void")
