#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00472db0 { uint8_t Unknown00[0x489]; uint8_t Value0489; uint8_t Unknown048a[0x16]; };
#pragma pack(pop)

// Stores the byte argument at +0x489 of the object and returns it in al
// (the callers test al afterwards).
// FUNCTION: GP4 0x00472db0
uint8_t __fastcall FUN_00472db0(Unknown_00472db0* self, void* /*edx*/, uint8_t value) {
    self->Value0489 = value;
    return value;
}
GP4_IMPL(0x00472db0, FUN_00472db0, "thiscall(ptr[0x4a0]:bytes, i32) -> u8")
