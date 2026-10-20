#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline int32_t absWrap(int32_t value) {
    return value < 0 ? static_cast<int32_t>(0u - static_cast<uint32_t>(value)) : value;
}

// Clamps the vector (eax, ebp, edx) to length ecx. When the largest |component| is at least ecx / 2
// the length is computed on the x87 stack as the rounded sqrt(edx^2 + ebp^2 + eax^2) (all in
// extended precision), and when it exceeds ecx every component is scaled by ecx / length (each
// product rounded back to an integer with fistp). DAT_006f2bf4 is the scratch dword every fild/fistp
// goes through. ebx and edi are preserved; the flags are those of the last compare.
// FUNCTION: GP4 0x00402cd0
void __cdecl FUN_00402cd0(gp4::Regs* r) {
    const int32_t x = static_cast<int32_t>(r->eax);
    const int32_t y = static_cast<int32_t>(r->ebp);
    const int32_t z = static_cast<int32_t>(r->edx);
    const int32_t limit = static_cast<int32_t>(r->ecx);

    int32_t largest = absWrap(x);
    int32_t candidate = absWrap(y);
    if (largest < candidate) {
        largest = candidate;
    }
    candidate = absWrap(z);
    if (largest < candidate) {
        largest = candidate;
    }
    const int32_t half = static_cast<int32_t>(static_cast<uint32_t>(limit) >> 1);
    if (largest < half) {
        r->eflags = (r->eflags & ~0x8d5u) | subFlags32(static_cast<uint32_t>(largest), static_cast<uint32_t>(half));
        return;
    }

    GP4_GLOBAL(int32_t, 0x006f2bf4) = z;
    const Ext zz = gp4::x87::ext_i32(z) * gp4::x87::ext_i32(z);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = y;
    const Ext yy = gp4::x87::ext_i32(y) * gp4::x87::ext_i32(y);
    const Ext zy = zz + yy;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = x;
    const Ext xx = gp4::x87::ext_i32(x) * gp4::x87::ext_i32(x);
    const Ext sum = zy + xx;
    const int32_t length = gp4::x87::fistp32(gp4::x87::fsqrt(sum));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = length;

    r->eflags = (r->eflags & ~0x8d5u) | subFlags32(static_cast<uint32_t>(length), static_cast<uint32_t>(limit));
    if (length <= limit) {
        return;
    }

    GP4_GLOBAL(int32_t, 0x006f2bf4) = limit;
    const Ext limitExt = gp4::x87::ext_i32(limit);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = length;
    const Ext ratio = limitExt / gp4::x87::ext_i32(length);

    GP4_GLOBAL(int32_t, 0x006f2bf4) = x;
    const int32_t scaledX = gp4::x87::fistp32(gp4::x87::ext_i32(x) * ratio);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaledX;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = y;
    const int32_t scaledY = gp4::x87::fistp32(gp4::x87::ext_i32(y) * ratio);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaledY;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = z;
    const int32_t scaledZ = gp4::x87::fistp32(gp4::x87::ext_i32(z) * ratio);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaledZ;

    r->eax = static_cast<uint32_t>(scaledX);
    r->ebp = static_cast<uint32_t>(scaledY);
    r->edx = static_cast<uint32_t>(scaledZ);
}
GP4_IMPL(0x00402cd0, FUN_00402cd0, "regs(eax:i32[-70000..70000], ebp:i32[-70000..70000], edx:i32[-70000..70000], ecx:i32[1..100000]) -> (eax, ebp, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
