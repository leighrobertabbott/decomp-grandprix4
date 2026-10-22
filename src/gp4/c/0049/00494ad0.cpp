#include <gp4/gp4.h>

// Vector add: out = other + self for three floats; the third component is computed first, as in the original.
// Leaves the output pointer in eax (callers read it).
// FUNCTION: GP4 0x00494ad0
float* __fastcall FUN_00494ad0(const float* self, void* /*edx*/, float* out, const float* other) {
    float z = other[2] + self[2];
    float y = other[1] + self[1];
    float x = other[0] + self[0];
    out[0] = x;
    out[1] = y;
    out[2] = z;
    return out;
}
GP4_IMPL(0x00494ad0, FUN_00494ad0, "thiscall(ptr[0x10]:f32, ptr[0x10]:f32, ptr[0x10]:f32) -> u32")
