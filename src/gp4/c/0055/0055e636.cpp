#include <gp4/gp4.h>
#include <gp4/x87.h>

// Transforms the 2D vector `in` by the first two rows of the 6-float matrix `m` (row stride 4
// floats: columns at +0/+4 and +1/+5), writing out.x = in.x*m[0] + in.y*m[4] and
// out.y = in.x*m[1] + in.y*m[5]. Each product and the sum stay at extended precision on the x87
// stack and are rounded once at the float store. Returns `out` in eax.
// FUNCTION: GP4 0x0055e636
int __stdcall FUN_0055e636(float* out, const float* in, const float* m) {
    using namespace gp4::x87;
    const float x = narrow_f32(ext_f32(m[4]) * ext_f32(in[1]) + ext_f32(in[0]) * ext_f32(m[0]));
    const float y = narrow_f32(ext_f32(m[1]) * ext_f32(in[0]) + ext_f32(m[5]) * ext_f32(in[1]));
    out[0] = x;
    out[1] = y;
    return static_cast<int>(reinterpret_cast<uintptr_t>(out));
}
GP4_IMPL(0x0055e636, FUN_0055e636, "stdcall(ptr[0x08]:f32, ptr[0x08]:f32, ptr[0x18]:f32) -> i32")
