#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005d9fe0 {
    uint32_t Value00;
    uint32_t Value04;
    uint32_t Value08;
    uint8_t Unknown0c[0x10];
    uint8_t Flag1c;
    uint8_t Unknown1d[3];
    int32_t Value20;
    uint8_t Unknown24[0x14];
    uint32_t Value38;
    uint32_t Value3c;
    uint32_t Value40;
    void* Pointer44;
    uint8_t Unknown48[0x104];
    uint32_t Value14c;
    uint8_t Flag150;
    uint8_t Unknown151[0xf];
};
#pragma pack(pop)

// Initialiser: sets the defaults (flag 1 at +0x1c, 1 at +0x38/+0x3c/+0x40, -1 at +0x20, 1.0f at +0x14c,
// zeros at +0/+4/+8/+0x150) and points +0x44 at the embedded block at +0x48. Returns the object.
// FUNCTION: GP4 0x005d9fe0
Unknown_005d9fe0* __fastcall FUN_005d9fe0(Unknown_005d9fe0* self, void* /*edx*/) {
    self->Flag1c = 1;
    self->Value38 = 1;
    self->Value3c = 1;
    self->Value40 = 1;
    self->Value04 = 0;
    self->Value08 = 0;
    self->Value20 = -1;
    self->Value00 = 0;
    self->Pointer44 = self->Unknown48;
    self->Value14c = 0x3f800000;
    self->Flag150 = 0;
    return self;
}
GP4_IMPL(0x005d9fe0, FUN_005d9fe0, "thiscall(ptr[0x160]:bytes) -> i32")
