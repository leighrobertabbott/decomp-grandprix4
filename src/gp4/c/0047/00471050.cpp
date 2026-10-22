#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_00471050 {
    uint8_t Unknown00[0xd84];
    float Value0d84;
};
#pragma pack(pop)

// Builds a perspective projection matrix (16 floats, row-major) from the value at +0xd84: zeroes
// the matrix, sets [10] = 1.0000625f, [11] = 1.0f, [14] = -0.50003123f, and the focal scales
// [0] = K1 / tan(f * K3 * K2) and [5] = K1 / tan(f * K3 * K0 * K2), where K0/K1/K2/K3 are the
// constants at 0x5f07b8 (float), 0x5f07c0 / 0x5f07c8 (double) and 0x5f07d0 (float).
// FUNCTION: GP4 0x00471050
void __fastcall FUN_00471050(Unknown_00471050* self, void* /*edx*/, uint32_t* matrix) {
    using namespace gp4::x87;
    Ext scaled = ext_f32(self->Value0d84) * ext_f32(GP4_GLOBAL(float, 0x005f07d0));
    Ext tanX = fptan(ext(GP4_GLOBAL(double, 0x005f07c8)) * scaled);
    for (int i = 0; i < 16; i++) {
        matrix[i] = 0;
    }
    matrix[10] = 0x3f80020c;
    matrix[14] = 0xbf00020c;
    matrix[11] = 0x3f800000;
    *reinterpret_cast<float*>(&matrix[0]) = narrow_f32(ext(GP4_GLOBAL(double, 0x005f07c0)) / tanX);
    Ext tanY = fptan(scaled * ext_f32(GP4_GLOBAL(float, 0x005f07b8)) * ext(GP4_GLOBAL(double, 0x005f07c8)));
    *reinterpret_cast<float*>(&matrix[5]) = narrow_f32(ext(GP4_GLOBAL(double, 0x005f07c0)) / tanY);
}
GP4_IMPL(0x00471050, FUN_00471050, "thiscall(ptr[0xd90]:{0xd84:f32[0.01..3]}, ptr[0x48]:bytes) -> void")
