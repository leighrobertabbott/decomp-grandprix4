#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Vec3_004468a0 {
    float X;
    float Y;
    float Z;
};
struct Plane_004468a0 {
    float A;
    float B;
    float C;
    float D;
};
struct Unknown_004468a0 {
    Plane_004468a0 Planes00[5];
    uint8_t Unknown50[0x10];
    int32_t CornerIndex60[5][2];
};
#pragma pack(pop)

// Signed distance-like plane value of box corner k (x from hi when bit 2 of k is set, y bit 1, z bit 0;
// the original builds the same eight corners as a table on its stack), summed as z*c + y*b + x*a + d
// on the x87 stack.
static inline bool CornerAbove(const Plane_004468a0* plane, const Vec3_004468a0* lo, const Vec3_004468a0* hi,
                               int32_t k) {
    const float x = (k & 4) ? hi->X : lo->X;
    const float y = (k & 2) ? hi->Y : lo->Y;
    const float z = (k & 1) ? hi->Z : lo->Z;
    return z * plane->C + y * plane->B + x * plane->A + plane->D > GP4_GLOBAL(float, 0x005f0420);
}

// Classifies the box spanned by lo/hi against five planes (a, b, c, d at +0, stride 0x10), each with
// a pair of corner indices (0..7, +0x60). Returns 0 as soon as the second corner of a plane lies above
// the limit 0x5f0420, 1 if only some first corners do, else 2.
// FUNCTION: GP4 0x004468a0
int __cdecl FUN_004468a0(const Unknown_004468a0* planes, const Vec3_004468a0* lo, const Vec3_004468a0* hi) {
    int result = 2;
    for (uint32_t i = 0; i < 5; i++) {
        const Plane_004468a0* plane = &planes->Planes00[i];
        if (CornerAbove(plane, lo, hi, planes->CornerIndex60[i][1])) {
            return 0;
        }
        if (CornerAbove(plane, lo, hi, planes->CornerIndex60[i][0])) {
            result = 1;
        }
    }
    return result;
}
GP4_IMPL(0x004468a0, FUN_004468a0, "cdecl(ptr[0x88]:{0*20/4:f32[-10..10], 0x60*10/4:i32[0..7]}, ptr[0x10]:f32[-10..10], ptr[0x10]:f32[-10..10]) -> i32")
