#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b7bb0 { int32_t Value00; int32_t Value04; int32_t Value08; int32_t Value0c; int32_t Value10; int32_t Value14; uint8_t Unknown18[0xe8]; };
#pragma pack(pop)

// Zeroes the six dwords at +0 .. +0x14 of the object and returns the object (`mov eax, ecx`).
// FUNCTION: GP4 0x005b7bb0
Unknown_005b7bb0* __fastcall FUN_005b7bb0(Unknown_005b7bb0* self, void* /*edx*/) {
    self->Value04 = 0;
    self->Value08 = 0;
    self->Value10 = 0;
    self->Value0c = 0;
    self->Value00 = 0;
    self->Value14 = 0;
    return self;
}
GP4_IMPL(0x005b7bb0, FUN_005b7bb0, "thiscall(ptr[0x100]:bytes) -> i32")
