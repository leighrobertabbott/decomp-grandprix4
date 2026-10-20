#include <gp4/gp4.h>
#include <gp4/x87.h>

// imul + shrd eax, edx, 0xe.
static inline int32_t mulShift14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
}

// One of four identical blocks: dword i of the arrays at 0x0060309c, 0x006030f4 and 0x00603104.
static inline void scaleEntry(int i) {
    using namespace gp4::x87;
    if (GP4_ARRAY(int32_t, 0x00603104)[i] == 0) return;

    int32_t weight = 0x7000 - (static_cast<int32_t>(static_cast<uint32_t>(GP4_ARRAY(int32_t, 0x0060309c)[i]) * 3u) >> 7);
    if (weight < 0) weight = 0;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = weight;

    const Ext product = ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)) * fabs(ext_i32(GP4_ARRAY(int32_t, 0x00603104)[i]));
    const Ext divisor = ext_i32(GP4_ARRAY(int32_t, 0x006030f4)[i]) * ext(GP4_GLOBAL(double, 0x006f2ac4));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(product / divisor);

    int32_t index = GP4_GLOBAL(int32_t, 0x006f2bf4);
    if (index < 0) index = 0;
    if (index > 0x100) index = 0x100;
    int32_t factor = GP4_ARRAY(uint16_t, 0x00603140)[index];
    if (GP4_ARRAY(int32_t, 0x00603104)[i] >= 0) {
        if (factor < GP4_GLOBAL(int32_t, 0x0060311c)) factor = GP4_GLOBAL(int32_t, 0x0060311c);
    } else {
        if (factor < GP4_GLOBAL(int32_t, 0x00603120)) factor = GP4_GLOBAL(int32_t, 0x00603120);
    }
    GP4_ARRAY(int32_t, 0x006030f4)[i] = mulShift14(factor, GP4_ARRAY(int32_t, 0x006030f4)[i]);
}

// FUNCTION: GP4 0x00404bcf
void __cdecl FUN_00404bcf() {
    if (GP4_GLOBAL(int32_t, 0x006030d0) == 0) return;
    scaleEntry(0);
    scaleEntry(1);
    scaleEntry(2);
    scaleEntry(3);
}
GP4_IMPL(0x00404bcf, FUN_00404bcf, "cdecl() -> void; globals=fuzz:i32")
