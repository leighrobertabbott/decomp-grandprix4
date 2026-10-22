#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004f2cf0 {
    uint8_t Unknown00[0x19c];
    float Scale19c[4];
    float Reduced1ac[3];
    float Scale1b8;
    uint8_t Unknown1bc[0x100];
};
#pragma pack(pop)

// Resets the 0x44-byte block at +0x19c: all dwords cleared, then the floats at +0x19c..+0x1a8 and
// +0x1b8 set to 1.0 and those at +0x1ac..+0x1b4 set to 0.75. Returns 0.
// FUNCTION: GP4 0x004f2cf0
int __fastcall FUN_004f2cf0(Unknown_004f2cf0* self, void* /*edx*/) {
    uint32_t* block = (uint32_t*)((uint8_t*)self + 0x19c);
    for (int i = 0; i < 0x11; i++) {
        block[i] = 0;
    }
    self->Scale19c[0] = 1.0f;
    self->Scale19c[1] = 1.0f;
    self->Scale19c[2] = 1.0f;
    self->Scale19c[3] = 1.0f;
    self->Scale1b8 = 1.0f;
    self->Reduced1ac[0] = 0.75f;
    self->Reduced1ac[1] = 0.75f;
    self->Reduced1ac[2] = 0.75f;
    return 0;
}
GP4_IMPL(0x004f2cf0, FUN_004f2cf0, "thiscall(ptr[0x300]:bytes) -> i32")
