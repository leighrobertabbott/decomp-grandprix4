#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_005d55e0 {
    uint8_t Unknown000[0x84];
    float Field84;
    float Field88;
    float Field8c;
    float Field90;
    uint8_t Unknown094[0x170 - 0x94];
    int32_t Mode170;
    uint8_t Unknown174[0x198 - 0x174];
    float X198;
    float Y19c;
    float Width1a0;
    float Height1a4;
    float MarginX1a8;
    float MarginY1ac;
    uint8_t Unknown1b0[0x1cc - 0x1b0];
    int32_t Result1cc;
};
#pragma pack(pop)

// C0 (a < b or unordered) / C3|C0 (a <= b or unordered) after FCOMP.
static inline bool c0(const gp4::x87::Ext& a, const gp4::x87::Ext& b) { return (gp4::x87::fcom(a, b) & 0x0100) != 0; }
static inline bool c30(const gp4::x87::Ext& a, const gp4::x87::Ext& b) { return (gp4::x87::fcom(a, b) & 0x4100) != 0; }

// Classifies the point (x, y): +0x1cc = 3 inside the rectangle (+0x198, +0x19c, +0x1a0, +0x1a4); then, depending on
// the orientation +0x170, = 1 or 2 when the point lies in the margin bands before or after it along that axis.
// FUNCTION: GP4 0x005d55e0
void __fastcall FUN_005d55e0(Unknown_005d55e0* self, void* /*edx*/, float x, float y) {
    using namespace gp4::x87;
    self->Result1cc = 0;
    if (!c0(ext_f32(x), ext_f32(self->X198)) &&
        !c0(ext_f32(self->Width1a0) + ext_f32(self->X198), ext_f32(x)) &&
        !c0(ext_f32(y), ext_f32(self->Y19c)) &&
        !c0(ext_f32(self->Height1a4) + ext_f32(self->Y19c), ext_f32(y)))
        self->Result1cc = 3;
    if (self->Mode170 == 1) {
        if (!c30(ext_f32(self->Field88) + ext_f32(self->MarginY1ac), ext_f32(y))) return;
        if (c0(ext_f32(self->Field90) + ext_f32(self->Field88) - ext_f32(self->MarginY1ac), ext_f32(y))) return;
        if (c0(ext_f32(x), ext_f32(self->X198))) {
            if (!c30(ext_f32(self->MarginX1a8) + ext_f32(self->Field84), ext_f32(x))) return;
            self->Result1cc = 1;
            return;
        }
        if (!c0(ext_f32(self->Width1a0) + ext_f32(self->X198), ext_f32(x))) return;
        if (c0(ext_f32(self->Field8c) + ext_f32(self->Field84) - ext_f32(self->MarginX1a8), ext_f32(x))) return;
        self->Result1cc = 2;
        return;
    }
    if (!c30(ext_f32(self->Field84) + ext_f32(self->MarginX1a8), ext_f32(x))) return;
    if (c0(ext_f32(self->Field8c) + ext_f32(self->Field84) - ext_f32(self->MarginX1a8), ext_f32(x))) return;
    if (c0(ext_f32(y), ext_f32(self->Y19c))) {
        if (!c30(ext_f32(self->MarginY1ac) + ext_f32(self->Field88), ext_f32(y))) return;
        self->Result1cc = 1;
        return;
    }
    if (!c0(ext_f32(self->Height1a4) + ext_f32(self->Y19c), ext_f32(y))) return;
    if (c0(ext_f32(self->Field90) + ext_f32(self->Field88) - ext_f32(self->MarginY1ac), ext_f32(y))) return;
    self->Result1cc = 2;
}
GP4_IMPL(0x005d55e0, FUN_005d55e0,
         "thiscall(ptr[0x1d8]:{0x84:f32[0..40], 0x88:f32[0..40], 0x8c:f32[0..100], 0x90:f32[0..100], 0x170:u32{=0,1,2}, 0x198:f32[0..60], 0x19c:f32[0..60], 0x1a0:f32[0..40], 0x1a4:f32[0..40], 0x1a8:f32[0..8], 0x1ac:f32[0..8]}, f32[0..100], f32[0..100]) -> void")
