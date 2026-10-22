#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a9370 {
    uint8_t Unknown00[0x13c];
    int32_t Value13c;
    int32_t Value140;
    int32_t Value144;
    int32_t Value148;
    int32_t Value14c;
    int32_t Value150;
};
#pragma pack(pop)

// Stores its six stack arguments into the consecutive dwords at +0x13c..+0x150 and returns 0.
// FUNCTION: GP4 0x005a9370
int32_t __fastcall FUN_005a9370(Unknown_005a9370* self, void* /*edx*/, int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f) {
    self->Value13c = a;
    self->Value140 = b;
    self->Value144 = c;
    self->Value148 = d;
    self->Value14c = e;
    self->Value150 = f;
    return 0;
}
GP4_IMPL(0x005a9370, FUN_005a9370, "thiscall(ptr[0x160]:{0x13c*6/4:i32}, i32, i32, i32, i32, i32, i32) -> i32")
