#include <gp4/gp4.h>

// 0x00401315: eax = 16-bit angle (sign-extended) of the vector (edx, eax); everything else is preserved.
static inline uint32_t angleOf(const gp4::Regs* r, uint32_t first, uint32_t second, uint32_t* flags) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = first;
    c.edx = second;
    gp4::call_regs(0x00401315, c);
    *flags = c.eflags;
    return c.eax;
}

// 0x0040113f: eax = integer square root of the unsigned 64-bit value edx:eax.
static inline uint32_t squareRoot64(const gp4::Regs* r, uint32_t low, uint32_t high) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = low;
    c.edx = high;
    gp4::call_regs(0x0040113f, c);
    return c.eax;
}

// Quaternion (eax, ebx, ecx, edx) = (w, x, y, z) as fixed-point words -> three angles:
//   eax = angle of (a^2 - b^2 - c^2 + d^2, 2(b*d - a*c))
//   ecx = angle of (a^2 - b^2 + c^2 - d^2, 2(a*d - b*c))
//   ebx = angle of (sqrt(2^56 - s^2), s) with s = 2(a*b + c*d)  (root argument clamped to the low word
//         when the high word would be negative)
//   edx = that square root. The flags are those of the last 0x00401315 call.
// The four inputs are also stored in 0x625a8c / 0x625a90 / 0x625a94 / 0x625a98.
// FUNCTION: GP4 0x0040e230
void __cdecl FUN_0040e230(gp4::Regs* r) {
    const uint32_t a = r->eax;
    const uint32_t b = r->ebx;
    const uint32_t c = r->ecx;
    const uint32_t d = r->edx;

    GP4_GLOBAL(uint32_t, 0x00625a8c) = a;
    GP4_GLOBAL(uint32_t, 0x00625a90) = b;
    GP4_GLOBAL(uint32_t, 0x00625a94) = c;
    GP4_GLOBAL(uint32_t, 0x00625a98) = d;

    const uint32_t aMinusB = a * a - b * b;
    const uint32_t cMinusD = c * c - d * d;

    uint32_t flags = 0;
    const uint32_t angle1 = angleOf(r, (b * d - a * c) << 1, aMinusB - cMinusD, &flags);
    const uint32_t angle2 = angleOf(r, (a * d - b * c) << 1, aMinusB + cMinusD, &flags);

    const uint32_t s = (c * d + a * b) << 1;
    const int64_t square = static_cast<int64_t>(static_cast<int32_t>(s)) * static_cast<int64_t>(static_cast<int32_t>(s));
    const uint32_t squareLow = static_cast<uint32_t>(square);
    const uint32_t squareHigh = static_cast<uint32_t>(static_cast<uint64_t>(square) >> 32);

    const uint32_t rootLow = 0u - squareLow;
    uint32_t rootHigh = 0x1000000u - squareHigh - (squareLow != 0u ? 1u : 0u);
    if (static_cast<int32_t>(rootHigh) < 0) {
        rootHigh = 0u;
    }
    const uint32_t root = squareRoot64(r, rootLow, rootHigh);
    const uint32_t angle3 = angleOf(r, s, root, &flags);

    r->eax = angle1;
    r->ebx = angle3;
    r->ecx = angle2;
    r->edx = root;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0040e230, FUN_0040e230, "regs(eax:i32[-0x4000..0x4000], ebx:i32[-0x4000..0x4000], ecx:i32[-0x4000..0x4000], edx:i32[-0x4000..0x4000]) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
