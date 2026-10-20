#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

// FCHS on an extended value: flips the sign bit only
static inline Ext negate(Ext v) {
    v.b[9] ^= 0x80;
    return v;
}

// One complex product through 0x00401d41 on the record at base + group: the integers at
// +0x634 / +0x63c are rotated by the pair (B*K, -A*K) built from +0x678 / +0x67c, K being the
// double at 0x006f29f4. Kept out of line so base + offset is not folded into absolute addresses.
static __declspec(noinline) void rotatePair(gp4::Regs& call, uint8_t* base, uint32_t group) {
    int32_t* re = reinterpret_cast<int32_t*>(base + group + 0x634);
    int32_t* im = reinterpret_cast<int32_t*>(base + group + 0x63c);
    const int32_t a = *reinterpret_cast<int32_t*>(base + group + 0x678);
    const int32_t b = *reinterpret_cast<int32_t*>(base + group + 0x67c);
    const Ext scale = gp4::x87::ext(GP4_GLOBAL(double, 0x006f29f4));
    const Ext negAScaled = negate(gp4::x87::ext_i32(a)) * scale;
    const Ext bScaled = gp4::x87::ext_i32(b) * scale;
    gp4::x87::set_in_st(call, 0, gp4::x87::ext_i32(*re));
    gp4::x87::set_in_st(call, 1, gp4::x87::ext_i32(*im));
    gp4::x87::set_in_st(call, 2, bScaled);
    gp4::x87::set_in_st(call, 3, negAScaled);
    call.st_in = 4;
    call.st_out = 2;
    gp4::call_regs(0x00401d41, call);
    *re = gp4::x87::fistp32(gp4::x87::out_st(call, 0));
    *im = gp4::x87::fistp32(gp4::x87::out_st(call, 1));
}

// FUNCTION: GP4 0x00417473
void __cdecl FUN_00417473(gp4::Regs* r) {
    uint8_t* base = GP4_ARRAY(uint8_t, 0x006380d8);
    gp4::Regs call = *r;
    call.ebx = 0x006380d8u;

    rotatePair(call, base, 0u);
    rotatePair(call, base, 0x300u);

    r->ebx = 0x006380d8u;
}
GP4_IMPL(0x00417473, FUN_00417473, "regs() -> (ebx); globals=fuzz:i32")
