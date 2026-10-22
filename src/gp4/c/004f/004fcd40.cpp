#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Plane_004fcd40 {
    float CoefA;   // multiplies the first float argument
    float CoefB;   // multiplies the second float argument, divisor of the result
    float CoefC;   // multiplies the third float argument
    float Offset;
};
struct Unknown_004fcd40 {
    uint8_t Unknown00[0x41c];
    Plane_004fcd40 Planes41c[8];
};
#pragma pack(pop)

// Moves the second coordinate onto plane `index` of the table at +0x41c (16-byte planes
// {a, b, c, d}): (-d - (z*c + x*a + y*b)) / b + y, left in st0 at extended precision.
// FUNCTION: GP4 0x004fcd40
double __fastcall FUN_004fcd40(Unknown_004fcd40* self, void* /*edx*/, int index, float x, float y, float z) {
    const Plane_004fcd40* plane = &self->Planes41c[index];
    return (-plane->Offset - (z * plane->CoefC + x * plane->CoefA + y * plane->CoefB)) / plane->CoefB + y;
}
GP4_IMPL(0x004fcd40, FUN_004fcd40, "thiscall(ptr[0x4a0]:f32, i32[0..7], f32, f32, f32) -> f64")
