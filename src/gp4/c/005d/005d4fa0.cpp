#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_005d4fa0 {
    uint8_t Unknown00[0x8c];
    float Value08c;
    float Value090;
    uint8_t Unknown094[0xdc];
    int32_t Value170;
    float Value174;
    uint8_t Unknown178[0x30];
    float Value1a8;
    float Value1ac;
};
#pragma pack(pop)

// Returns the float at +0x174 divided by (limit - 2 * margin), left in st0 in extended precision.
// Mode 1 at +0x170 uses the pair +0x8c / +0x1a8, otherwise +0x90 / +0x1ac.
// FUNCTION: GP4 0x005d4fa0
double __fastcall FUN_005d4fa0(Unknown_005d4fa0* self, void* /*edx*/) {
    using namespace gp4::x87;
    if (self->Value170 == 1) {
        Ext twice = ext_f32(self->Value1a8);
        twice = twice + twice;
        return narrow(ext_f32(self->Value174) / (ext_f32(self->Value08c) - twice));
    }
    Ext twice = ext_f32(self->Value1ac);
    twice = twice + twice;
    return narrow(ext_f32(self->Value174) / (ext_f32(self->Value090) - twice));
}
GP4_IMPL(0x005d4fa0, FUN_005d4fa0, "thiscall(ptr[0x1c0]:{0x8c:f32, 0x90:f32, 0x170:u32{=0,1,1,2}, 0x174:f32, 0x1a8:f32, 0x1ac:f32}) -> f64; tol=1e-12")
