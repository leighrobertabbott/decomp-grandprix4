#include <gp4/gp4.h>

static inline int32_t mul32(int32_t x, int32_t y) {
    return static_cast<int32_t>(static_cast<uint32_t>(x) * static_cast<uint32_t>(y));
}

static inline int32_t negate32(int32_t v) { return static_cast<int32_t>(0u - static_cast<uint32_t>(v)); }

// FUNCTION: GP4 0x004016ba
// Builds a 3x3 rotation matrix in 2.14 fixed point from three (sin, cos) pairs at
// 0x00623ec4/c8 (cos, sin), 0x00623ecc/d0 and 0x00623ed4/d8 into 0x00623e3c..0x00623e5c.
// Every term is the low dword of an imul, summed and then shifted right by 14 (sar eax, 14).
// Leaves edx = high dword of the last imul, ebp = the previous imul's low dword and the flags of
// the final sar.
void __cdecl FUN_004016ba(gp4::Regs* r) {
    const int32_t a = GP4_GLOBAL(int32_t, 0x00623ec4);
    const int32_t b = GP4_GLOBAL(int32_t, 0x00623ec8);
    const int32_t c = GP4_GLOBAL(int32_t, 0x00623ecc);
    const int32_t d = GP4_GLOBAL(int32_t, 0x00623ed0);
    const int32_t e = GP4_GLOBAL(int32_t, 0x00623ed4);
    const int32_t f = GP4_GLOBAL(int32_t, 0x00623ed8);

    const int32_t ce = mul32(c, e) >> 14;       // esi
    const int32_t cf = mul32(c, f) >> 14;       // edi

    GP4_GLOBAL(int32_t, 0x00623e3c) = (mul32(b, f) + mul32(a, ce)) >> 14;
    GP4_GLOBAL(int32_t, 0x00623e40) = negate32(mul32(d, e) >> 14);
    GP4_GLOBAL(int32_t, 0x00623e44) = (mul32(b, ce) - mul32(a, f)) >> 14;
    GP4_GLOBAL(int32_t, 0x00623e48) = (mul32(b, e) - mul32(a, cf)) >> 14;
    GP4_GLOBAL(int32_t, 0x00623e4c) = mul32(d, f) >> 14;
    const int32_t ae = mul32(a, e);             // ebp
    GP4_GLOBAL(int32_t, 0x00623e50) = negate32((mul32(b, cf) + ae) >> 14);
    GP4_GLOBAL(int32_t, 0x00623e54) = mul32(a, d) >> 14;
    GP4_GLOBAL(int32_t, 0x00623e58) = c;

    // Last term: eax = b * d, edx = high dword, then sar eax, 14.
    const int64_t product = static_cast<int64_t>(b) * static_cast<int64_t>(d);
    const uint32_t low = static_cast<uint32_t>(product);
    const int32_t result = static_cast<int32_t>(low) >> 14;
    GP4_GLOBAL(int32_t, 0x00623e5c) = result;

    uint32_t parity = static_cast<uint32_t>(result) & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    // sar by 14: CF = last bit shifted out, OF = AF = 0.
    const uint32_t flags = ((low >> 13) & 1u) | ((~parity & 1u) << 2)
        | (result == 0 ? 0x40u : 0u) | (result < 0 ? 0x80u : 0u);

    r->eax = static_cast<uint32_t>(result);
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->ebp = static_cast<uint32_t>(ae);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004016ba, FUN_004016ba, "regs() -> (eax,edx,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32")
