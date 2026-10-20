#include <gp4/gp4.h>
#include <gp4/x87.h>

struct Vec3_0055e32b {
    float X00;
    uint32_t Y04;
    uint32_t Z08;
};

struct Plane_0055e32b {
    float X00;
    uint32_t Y04;
    uint32_t Z08;
    float D0c;
};

// Builds a plane (normal, d) from a point and a normal: out.xyz = normal.xyz (x through the x87,
// y and z as raw dwords, like the original) and out.d = -(point . normal), the products and sums
// kept at extended precision (y*y' + z*z' first, then x*x') and rounded once at the final store.

// FUNCTION: GP4 0x0055e32b
void __stdcall FUN_0055e32b(Plane_0055e32b* out, const Vec3_0055e32b* point, const Vec3_0055e32b* normal) {
    using namespace gp4::x87;
    out->X00 = narrow_f32(ext_f32(normal->X00));
    out->Y04 = normal->Y04;
    out->Z08 = normal->Z08;
    Ext sum = ext_f32(*reinterpret_cast<const float*>(&point->Y04)) * ext_f32(*reinterpret_cast<const float*>(&normal->Y04));
    sum = sum + ext_f32(*reinterpret_cast<const float*>(&point->Z08)) * ext_f32(*reinterpret_cast<const float*>(&normal->Z08));
    sum = sum + ext_f32(point->X00) * ext_f32(normal->X00);
    out->D0c = narrow_f32(fchs(sum));
}
GP4_IMPL(0x0055e32b, FUN_0055e32b, "stdcall(ptr[0x10]:f32, ptr[0x0c]:f32, ptr[0x0c]:f32) -> void")
