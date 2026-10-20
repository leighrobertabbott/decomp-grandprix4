#include <gp4/gp4.h>

// From the vector (x, y, z) = ([0x009630b4], [0x009630b8], [0x009630bc]): the horizontal length
// h = isqrt(x*x + z*z) (0x0040113f, 0 -> 1) gives the Q14 ratios [0x008755d8] = (x << 14) / h and
// [0x008755dc] = (z << 14) / h; the full length l = isqrt(x*x + y*y + z*z) (0 -> 1) gives
// [0x008755e0] = q = (y << 14) / l and [0x008755e4] = isqrt(max(0, 2^28 - q*q)). The word angle
// [0x00891510] then gives [0x008755ec] = table(angle) and [0x008755e8] = table(0x4000 - angle) via
// 0x0040119c. Register routine: eax/edx = those two (sign-extended), ebx = l (or 1), ebp = high
// half of q*q; the flags are those of the last 0x0040119c call.
// FUNCTION: GP4 0x005271a7
void __cdecl FUN_005271a7(gp4::Regs* r) {
    const int32_t x = GP4_GLOBAL(int32_t, 0x009630b4);
    const int32_t z = GP4_GLOBAL(int32_t, 0x009630bc);
    const uint64_t horiz = static_cast<uint64_t>(static_cast<int64_t>(z) * z) +
                           static_cast<uint64_t>(static_cast<int64_t>(x) * x);

    gp4::Regs c = *r;
    c.eax = static_cast<uint32_t>(horiz);
    c.edx = static_cast<uint32_t>(horiz >> 32);
    gp4::call_regs(0x0040113f, c);
    int32_t h = static_cast<int32_t>(c.eax);
    if (h == 0) h = 1;
    int32_t rem;
    GP4_GLOBAL(int32_t, 0x008755d8) = gp4::idiv64(static_cast<int64_t>(x) << 14, h, &rem);
    GP4_GLOBAL(int32_t, 0x008755dc) = gp4::idiv64(static_cast<int64_t>(z) << 14, h, &rem);

    const int32_t y = GP4_GLOBAL(int32_t, 0x009630b8);
    const uint64_t full = static_cast<uint64_t>(static_cast<int64_t>(y) * y) + horiz;
    c.eax = static_cast<uint32_t>(full);
    c.edx = static_cast<uint32_t>(full >> 32);
    gp4::call_regs(0x0040113f, c);
    int32_t l = static_cast<int32_t>(c.eax);
    if (l == 0) l = 1;
    const int32_t q = gp4::idiv64(static_cast<int64_t>(y) << 14, l, &rem);
    GP4_GLOBAL(int32_t, 0x008755e0) = q;

    const int64_t qq = static_cast<int64_t>(q) * q;
    int64_t rest = 0x10000000LL - qq;
    if (rest < 0) rest = 0;
    c.eax = static_cast<uint32_t>(static_cast<uint64_t>(rest));
    c.edx = static_cast<uint32_t>(static_cast<uint64_t>(rest) >> 32);
    gp4::call_regs(0x0040113f, c);
    GP4_GLOBAL(uint32_t, 0x008755e4) = c.eax;

    const uint16_t angle = GP4_GLOBAL(uint16_t, 0x00891510);
    c.eax = (c.eax & 0xffff0000u) | angle;
    gp4::call_regs(0x0040119c, c);
    const int32_t first = static_cast<int16_t>(c.eax);
    GP4_GLOBAL(int32_t, 0x008755ec) = first;

    c.eax = (static_cast<uint32_t>(first) & 0xffff0000u) | static_cast<uint16_t>(0x4000 - angle);
    gp4::call_regs(0x0040119c, c);
    const int32_t second = static_cast<int16_t>(c.eax);
    GP4_GLOBAL(int32_t, 0x008755e8) = second;

    r->eax = static_cast<uint32_t>(first);
    r->edx = static_cast<uint32_t>(second);
    r->ebx = static_cast<uint32_t>(l);
    r->ebp = static_cast<uint32_t>(static_cast<uint64_t>(qq) >> 32);
    r->eflags = c.eflags;
}
GP4_IMPL(0x005271a7, FUN_005271a7, "regs() -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
