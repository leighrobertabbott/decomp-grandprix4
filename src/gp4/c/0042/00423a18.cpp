#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

// 0x00401ccc with st0 = limit, st1 = divisor, st2 = dividend: the clamped ratio (80-bit).
static inline Ext clampedRatio(const gp4::Regs& state, const Ext& limit, const Ext& divisor, const Ext& dividend) {
    gp4::Regs callee = state;
    callee.st_xin_mask = 0;
    callee.st_xout_mask = 0;
    callee.st_in = 3;
    callee.st_out = 1;
    gp4::x87::set_in_st(callee, 0, limit);
    gp4::x87::set_in_st(callee, 1, divisor);
    gp4::x87::set_in_st(callee, 2, dividend);
    gp4::call_regs(0x00401ccc, callee);
    return gp4::x87::out_st(callee, 0);
}

// fild value; fmul [0x6f2b2c]; fild [0x639630]; fild [0x608028]; 0x401ccc; fistp.
static inline int32_t scaledRatio(const gp4::Regs& state, int32_t value) {
    using namespace gp4::x87;
    const Ext dividend = ext_i32(value) * ext(GP4_GLOBAL(double, 0x006f2b2c));
    const Ext divisor = ext_i32(GP4_GLOBAL(int32_t, 0x00639630));
    const Ext limit = ext_i32(GP4_GLOBAL(int32_t, 0x00608028));
    return fistp32(clampedRatio(state, limit, divisor, dividend));
}

// fild qword total; fmul [0x6f2be4]; fmul [0x6f2b8c]; fild count; fild [0x608258]; fmulp; fild [0x60802c]; 0x401ccc.
static inline Ext angleRatio(const gp4::Regs& state, int64_t total, int32_t count) {
    using namespace gp4::x87;
    const Ext dividend = ext_i64(total) * ext(GP4_GLOBAL(double, 0x006f2be4)) * ext(GP4_GLOBAL(double, 0x006f2b8c));
    const Ext divisor = ext_i32(count) * ext_i32(GP4_GLOBAL(int32_t, 0x00608258));
    const Ext limit = ext_i32(GP4_GLOBAL(int32_t, 0x0060802c));
    return clampedRatio(state, limit, divisor, dividend);
}

// The fixed body at 0x00622e98 (kept out of line so its fields are addressed through the pointer, as the original
// does with edx).
static __declspec(noinline) uint8_t* fixedBody() {
    return GP4_ARRAY(uint8_t, 0x00622e98);
}

// When either 0x608274 or 0x60828c is set, divides the six accumulators 0x608180..0x608194 (times [0x6f2b2c]) by
// [0x639630] with the clamp [0x608028] (0x401ccc) into 0x607ff4/f8/fc and 0x607fe8/ec/f0, and the three qword
// accumulators 0x608168/70/78 (times [0x6f2be4] * [0x6f2b8c]) by [0x6097c0+4i] * [0x608258] with the clamp [0x60802c]
// into 0x60822c/30/34 and 0x623e18/1c/20 (the last adjusted by 0x4239dc when byte 0x60827d is set); that vector is
// rotated by the body 0x622e98 (0x401f02) into 0x623e24/28/2c = 0x6081d4/d8/dc. Otherwise all those outputs are zero.
// Then 0x423987 runs. eax = [0x623e2c] and edx = 0x622e98 on the computing path.
// FUNCTION: GP4 0x00423a18
void __cdecl FUN_00423a18(gp4::Regs* r) {
    using namespace gp4::x87;
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    if (GP4_GLOBAL(uint32_t, 0x00608274) == 0u && GP4_GLOBAL(uint32_t, 0x0060828c) == 0u) {
        GP4_GLOBAL(uint32_t, 0x00607fe8) = 0;
        GP4_GLOBAL(uint32_t, 0x00607fec) = 0;
        GP4_GLOBAL(uint32_t, 0x00607ff0) = 0;
        GP4_GLOBAL(uint32_t, 0x00607ff4) = 0;
        GP4_GLOBAL(uint32_t, 0x00607ff8) = 0;
        GP4_GLOBAL(uint32_t, 0x00607ffc) = 0;
        GP4_GLOBAL(uint32_t, 0x006081d4) = 0;
        GP4_GLOBAL(uint32_t, 0x006081d8) = 0;
        GP4_GLOBAL(uint32_t, 0x006081dc) = 0;
    } else {
        GP4_GLOBAL(int32_t, 0x00607ff4) = scaledRatio(s, GP4_GLOBAL(int32_t, 0x00608180));
        GP4_GLOBAL(int32_t, 0x00607ff8) = scaledRatio(s, GP4_GLOBAL(int32_t, 0x00608184));
        GP4_GLOBAL(int32_t, 0x00607ffc) = scaledRatio(s, GP4_GLOBAL(int32_t, 0x00608188));
        GP4_GLOBAL(int32_t, 0x00607fe8) = scaledRatio(s, GP4_GLOBAL(int32_t, 0x0060818c));
        GP4_GLOBAL(int32_t, 0x00607fec) = scaledRatio(s, GP4_GLOBAL(int32_t, 0x00608190));
        GP4_GLOBAL(int32_t, 0x00607ff0) = scaledRatio(s, GP4_GLOBAL(int32_t, 0x00608194));

        const Ext x = angleRatio(s, GP4_GLOBAL(int64_t, 0x00608168), GP4_GLOBAL(int32_t, 0x006097c0));
        GP4_GLOBAL(int32_t, 0x0060822c) = fistp32(x);
        GP4_GLOBAL(int32_t, 0x00623e18) = fistp32(x);
        const Ext y = angleRatio(s, GP4_GLOBAL(int64_t, 0x00608170), GP4_GLOBAL(int32_t, 0x006097c4));
        GP4_GLOBAL(int32_t, 0x00608230) = fistp32(y);
        GP4_GLOBAL(int32_t, 0x00623e1c) = fistp32(y);
        const Ext z = angleRatio(s, GP4_GLOBAL(int64_t, 0x00608178), GP4_GLOBAL(int32_t, 0x006097c8));
        GP4_GLOBAL(int32_t, 0x00608234) = fistp32(z);
        GP4_GLOBAL(int32_t, 0x00623e20) = fistp32(z);

        s.eax = GP4_GLOBAL(uint32_t, 0x00608234);
        if (GP4_GLOBAL(uint8_t, 0x0060827d) != 0u) {
            gp4::call_regs(0x004239dc, s);
        }
        GP4_GLOBAL(uint32_t, 0x00608234) = s.eax;
        GP4_GLOBAL(uint32_t, 0x00623e20) = s.eax;

        uint8_t* body = fixedBody();
        s.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
        GP4_FIELD(double, body, 0xb8) = narrow(ext_i32(GP4_GLOBAL(int32_t, 0x00623e18)));
        GP4_FIELD(double, body, 0xc0) = narrow(ext_i32(GP4_GLOBAL(int32_t, 0x00623e1c)));
        GP4_FIELD(double, body, 0xc8) = narrow(ext_i32(GP4_GLOBAL(int32_t, 0x00623e20)));
        gp4::call_regs(0x00401f02, s);
        GP4_GLOBAL(int32_t, 0x00623e24) = fistp32(ext(GP4_FIELD(double, body, 0xa0)));
        GP4_GLOBAL(int32_t, 0x00623e28) = fistp32(ext(GP4_FIELD(double, body, 0xa8)));
        GP4_GLOBAL(int32_t, 0x00623e2c) = fistp32(ext(GP4_FIELD(double, body, 0xb0)));

        GP4_GLOBAL(uint32_t, 0x006081d4) = GP4_GLOBAL(uint32_t, 0x00623e24);
        GP4_GLOBAL(uint32_t, 0x006081d8) = GP4_GLOBAL(uint32_t, 0x00623e28);
        s.eax = GP4_GLOBAL(uint32_t, 0x00623e2c);
        GP4_GLOBAL(uint32_t, 0x006081dc) = s.eax;
    }

    gp4::call_regs(0x00423987, s);

    r->eax = s.eax;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->ebp = s.ebp;
    r->eflags = s.eflags;
}
GP4_IMPL(0x00423a18, FUN_00423a18, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860+state_00423987")
