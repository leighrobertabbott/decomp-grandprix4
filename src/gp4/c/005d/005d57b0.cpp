#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005d57b0 {
    uint8_t Unknown00[0x1b4];
    float Numerator1b4;
    float Denominator1b8;
};
#pragma pack(pop)

// Returns numerator / denominator * scale (kept in extended precision on the x87 stack); when either
// operand compares equal to (or unordered with) the float constant at 0x005fa0c4 the constant is
// returned instead.
// FUNCTION: GP4 0x005d57b0
double __fastcall FUN_005d57b0(Unknown_005d57b0* self, void* /*edx*/, float scale) {
    float zero = GP4_GLOBAL(float, 0x005fa0c4);
    double result = zero;
    if ((self->Numerator1b4 < zero || self->Numerator1b4 > zero) && (self->Denominator1b8 < zero || self->Denominator1b8 > zero)) {
        result = self->Numerator1b4 / (double)self->Denominator1b8 * scale;
    }
    return result;
}
GP4_IMPL(0x005d57b0, FUN_005d57b0, "thiscall(ptr[0x1c0]:{0x1b4:f32, 0x1b8:f32}, f32) -> f64")
