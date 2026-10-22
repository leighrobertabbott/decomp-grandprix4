#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bb0e0 { uint32_t Value00; uint8_t Byte04; uint8_t Unknown05[3]; uint32_t Value08; };
#pragma pack(pop)

// Constructor-like clear: zeroes +8 (dword), +0 (dword) and +4 (byte), returns this.
// FUNCTION: GP4 0x005bb0e0
Unknown_005bb0e0* __fastcall FUN_005bb0e0(Unknown_005bb0e0* self, void* /*edx*/) {
    self->Value08 = 0;
    self->Value00 = 0;
    self->Byte04 = 0;
    return self;
}
GP4_IMPL(0x005bb0e0, FUN_005bb0e0, "thiscall(ptr[0x18]:bytes) -> i32")
