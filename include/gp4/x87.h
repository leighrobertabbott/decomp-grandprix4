// x87.h - bit-exact x87 primitives.
//
// GP4's simulation is x87 code. A C library sin()/cos()/sqrt() or a C cast to
// int does NOT produce the same bits as the FSIN/FCOS/FSQRT/FISTP instructions
// the original executes. Reconstructions must use these helpers wherever the
// original uses the instruction, so the oracle can demand exact equality.
#pragma once
#include <stdint.h>

namespace gp4 { namespace x87 {

inline double fsin(double x) { double r; __asm { fld x  } __asm { fsin } __asm { fstp r } return r; }
inline double fcos(double x) { double r; __asm { fld x  } __asm { fcos } __asm { fstp r } return r; }
inline void fsincos(double x, double* s, double* c) {
    double sv, cv;         // (not "ss"/"cs": those are segment register names in __asm)
    __asm { fld x }
    __asm { fsincos }      // st0 = cos, st1 = sin
    __asm { fstp cv }
    __asm { fstp sv }
    *s = sv; *c = cv;
}
inline double fsqrt(double x) { double r; __asm { fld x } __asm { fsqrt } __asm { fstp r } return r; }
inline double fabs_(double x) { double r; __asm { fld x } __asm { fabs } __asm { fstp r } return r; }
// atan2(y, x) exactly as FPATAN computes it: st1 = y, st0 = x
inline double fpatan(double y, double x) {
    double r;
    __asm { fld y }
    __asm { fld x }
    __asm { fpatan }
    __asm { fstp r }
    return r;
}
inline double fptan(double x) { double r; __asm { fld x } __asm { fptan } __asm { fstp st(0) } __asm { fstp r } return r; }
inline double frndint(double x) { double r; __asm { fld x } __asm { frndint } __asm { fstp r } return r; }
inline double fscale(double x, double e) { double r; __asm { fld e } __asm { fld x } __asm { fscale } __asm { fstp st(1) } __asm { fstp r } return r; }
inline double fprem(double x, double y) {
    double r;
    __asm { fld y }
    __asm { fld x }
again:
    __asm { fprem }
    __asm { fnstsw ax }
    __asm { test ah, 4 }
    __asm { jnz again }
    __asm { fstp st(1) }
    __asm { fstp r }
    return r;
}
// FISTP with the CURRENT rounding mode (round-to-nearest by default) - what
// hand-written asm does. A C cast truncates instead.
inline int32_t fistp32(double x) { int32_t r; __asm { fld x } __asm { fistp r } return r; }
inline int16_t fistp16(double x) { int16_t r; __asm { fld x } __asm { fistp r } return r; }
inline int64_t fistp64(double x) { int64_t r; __asm { fld x } __asm { fistp r } return r; }

inline uint16_t get_cw() { uint16_t cw; __asm { fnstcw cw } return cw; }
inline void set_cw(uint16_t cw) { __asm { fldcw cw } }

// ---- 80-bit intermediates -------------------------------------------------
// When the original keeps values on the x87 stack across several instructions
// (no store to a float/double slot in between), every intermediate stays in
// extended precision. Ext holds one such value; it round-trips through
// FLD/FSTP TBYTE, which is lossless, so each operation rounds exactly once,
// like the instruction. Use ext() to load a double, narrow() where the original
// finally stores to a qword slot. Operands appear in the original's order.
struct Ext { uint8_t b[10]; };

inline Ext ext(double x) { Ext r; Ext* p = &r; __asm { fld x } __asm { mov eax, p } __asm { fstp tbyte ptr [eax] } return r; }
inline double narrow(const Ext& a) { double r; const Ext* p = &a; __asm { mov eax, p } __asm { fld tbyte ptr [eax] } __asm { fstp r } return r; }
inline float narrow_f32(const Ext& a) { float r; const Ext* p = &a; __asm { mov eax, p } __asm { fld tbyte ptr [eax] } __asm { fstp r } return r; }
inline Ext operator+(const Ext& a, const Ext& b) {
    Ext r; const Ext* pa = &a; const Ext* pb = &b; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { mov eax, pb } __asm { fld tbyte ptr [eax] }
    __asm { faddp st(1), st } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
inline Ext operator-(const Ext& a, const Ext& b) {
    Ext r; const Ext* pa = &a; const Ext* pb = &b; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { mov eax, pb } __asm { fld tbyte ptr [eax] }
    __asm { fsubp st(1), st } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
inline Ext operator*(const Ext& a, const Ext& b) {
    Ext r; const Ext* pa = &a; const Ext* pb = &b; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { mov eax, pb } __asm { fld tbyte ptr [eax] }
    __asm { fmulp st(1), st } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
inline Ext operator/(const Ext& a, const Ext& b) {
    Ext r; const Ext* pa = &a; const Ext* pb = &b; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { mov eax, pb } __asm { fld tbyte ptr [eax] }
    __asm { fdivp st(1), st } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
inline Ext fsqrt(const Ext& a) {
    Ext r; const Ext* pa = &a; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { fsqrt } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
// atan2(y, x) on extended operands: st1 = y, st0 = x
inline Ext fpatan(const Ext& y, const Ext& x) {
    Ext r; const Ext* py = &y; const Ext* px = &x; Ext* pr = &r;
    __asm { mov eax, py } __asm { fld tbyte ptr [eax] } __asm { mov eax, px } __asm { fld tbyte ptr [eax] }
    __asm { fpatan } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}

// Unary instructions on an extended value (st0 in, st0 out).
inline Ext fsin(const Ext& a) {
    Ext r; const Ext* pa = &a; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { fsin } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
inline Ext fcos(const Ext& a) {
    Ext r; const Ext* pa = &a; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { fcos } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
inline Ext fptan(const Ext& a) {   // FPTAN pushes 1.0; the tangent is below it
    Ext r; const Ext* pa = &a; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { fptan } __asm { fstp st(0) } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
inline Ext frndint(const Ext& a) {   // current rounding mode
    Ext r; const Ext* pa = &a; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { frndint } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
inline Ext f2xm1(const Ext& a) {   // 2^x - 1 for |x| <= 1
    Ext r; const Ext* pa = &a; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { f2xm1 } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
inline Ext fabs(const Ext& a) {
    Ext r; const Ext* pa = &a; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { fabs } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
inline Ext fchs(const Ext& a) {
    Ext r; const Ext* pa = &a; Ext* pr = &r;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { fchs } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}

inline void fsincos(const Ext& a, Ext* s, Ext* c) {
    const Ext* pa = &a;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { fsincos }   // st0 = cos, st1 = sin
    __asm { mov eax, c } __asm { fstp tbyte ptr [eax] } __asm { mov eax, s } __asm { fstp tbyte ptr [eax] }
}
// FSCALE: st0 = x * 2^trunc(e), with st1 = e
inline Ext fscale(const Ext& x, const Ext& e) {
    Ext r; const Ext* px = &x; const Ext* pe = &e; Ext* pr = &r;
    __asm { mov eax, pe } __asm { fld tbyte ptr [eax] } __asm { mov eax, px } __asm { fld tbyte ptr [eax] }
    __asm { fscale } __asm { fstp st(1) } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
// FYL2X: y * log2(x), with st1 = y, st0 = x
inline Ext fyl2x(const Ext& y, const Ext& x) {
    Ext r; const Ext* py = &y; const Ext* px = &x; Ext* pr = &r;
    __asm { mov eax, py } __asm { fld tbyte ptr [eax] } __asm { mov eax, px } __asm { fld tbyte ptr [eax] }
    __asm { fyl2x } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
// FPREM until complete: partial remainder of x by y (st0 = x, st1 = y)
inline Ext fprem(const Ext& x, const Ext& y) {
    Ext r; const Ext* px = &x; const Ext* py = &y; Ext* pr = &r;
    __asm { mov eax, py } __asm { fld tbyte ptr [eax] } __asm { mov eax, px } __asm { fld tbyte ptr [eax] }
again_ext:
    __asm { fprem } __asm { fnstsw ax } __asm { test ah, 4 } __asm { jnz again_ext }
    __asm { fstp st(1) } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] }
    return r;
}
// Constants exactly as the load-constant instructions push them.
inline Ext ext_one() { Ext r; Ext* pr = &r; __asm { fld1 } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] } return r; }
inline Ext ext_zero() { Ext r; Ext* pr = &r; __asm { fldz } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] } return r; }
inline Ext ext_pi() { Ext r; Ext* pr = &r; __asm { fldpi } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] } return r; }
inline Ext ext_l2e() { Ext r; Ext* pr = &r; __asm { fldl2e } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] } return r; }
inline Ext ext_l2t() { Ext r; Ext* pr = &r; __asm { fldl2t } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] } return r; }
inline Ext ext_lg2() { Ext r; Ext* pr = &r; __asm { fldlg2 } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] } return r; }
inline Ext ext_ln2() { Ext r; Ext* pr = &r; __asm { fldln2 } __asm { mov eax, pr } __asm { fstp tbyte ptr [eax] } return r; }
inline Ext ext_i16(int16_t x) { Ext r; Ext* p = &r; __asm { fild x } __asm { mov eax, p } __asm { fstp tbyte ptr [eax] } return r; }
inline Ext ext_i64(int64_t x) { Ext r; Ext* p = &r; __asm { fild x } __asm { mov eax, p } __asm { fstp tbyte ptr [eax] } return r; }

// Comparisons: the x87 status word bits C3 (0x4000), C2 (0x0400), C0 (0x0100)
// after comparing st0 = a with st1 = b. a > b: none; a < b: C0; equal: C3;
// unordered: C3|C2|C0. After FNSTSW AX / SAHF: CF = C0, PF = C2, ZF = C3.
inline uint16_t fucom(const Ext& a, const Ext& b) {
    uint16_t sw; const Ext* pa = &a; const Ext* pb = &b;
    __asm { mov eax, pb } __asm { fld tbyte ptr [eax] } __asm { mov eax, pa } __asm { fld tbyte ptr [eax] }
    __asm { fucompp } __asm { fnstsw ax } __asm { mov sw, ax }
    return static_cast<uint16_t>(sw & 0x4500u);
}
inline uint16_t fcom(const Ext& a, const Ext& b) {     // signalling compare (FCOM/FCOMP/FCOMPP)
    uint16_t sw; const Ext* pa = &a; const Ext* pb = &b;
    __asm { mov eax, pb } __asm { fld tbyte ptr [eax] } __asm { mov eax, pa } __asm { fld tbyte ptr [eax] }
    __asm { fcompp } __asm { fnstsw ax } __asm { mov sw, ax }
    return static_cast<uint16_t>(sw & 0x4500u);
}
inline uint16_t ftst(const Ext& a) {                   // compare with +0.0
    uint16_t sw; const Ext* pa = &a;
    __asm { mov eax, pa } __asm { fld tbyte ptr [eax] } __asm { ftst } __asm { fnstsw ax } __asm { mov sw, ax }
    __asm { fstp st(0) }
    return static_cast<uint16_t>(sw & 0x4500u);
}

// FISTP straight from an extended value (e.g. an FILD/FMUL product the original
// never stores): one rounding, in the current mode, like the instruction.
inline int32_t fistp32(const Ext& a) { int32_t r; const Ext* p = &a; __asm { mov eax, p } __asm { fld tbyte ptr [eax] } __asm { fistp r } return r; }
inline int16_t fistp16(const Ext& a) { int16_t r; const Ext* p = &a; __asm { mov eax, p } __asm { fld tbyte ptr [eax] } __asm { fistp r } return r; }
inline Ext ext_i32(int32_t x) { Ext r; Ext* p = &r; __asm { fild x } __asm { mov eax, p } __asm { fstp tbyte ptr [eax] } return r; }
inline Ext ext_f32(float x) { Ext r; Ext* p = &r; __asm { fld x } __asm { mov eax, p } __asm { fstp tbyte ptr [eax] } return r; }

}}  // namespace gp4::x87

// ---- 80-bit values in the register file (regs convention) ---------------
// in_st(r, k): x87 input k at full precision (the double st[k] when the caller
// only supplied a double). set_out_st(r, k, v): x87 output k at full precision
// (also stores the rounded double). out_st(r, k): output k of an original
// routine called through gp4::call_regs. set_in_st(r, k, v): input k for it.
#include <gp4/gp4.h>
namespace gp4 { namespace x87 {
inline Ext in_st(const Regs* r, int k) {
    if ((r->st_xin_mask >> k) & 1u) { Ext v; for (int i = 0; i < 10; ++i) v.b[i] = r->st_xin[k][i]; return v; }
    return ext(r->st[k]);
}
inline void set_out_st(Regs* r, int k, const Ext& v) {
    for (int i = 0; i < 10; ++i) r->st_xout[k][i] = v.b[i];
    r->st_xout_mask |= 1u << k;
    r->st[k] = narrow(v);
}
inline Ext out_st(const Regs& r, int k) {
    if ((r.st_xout_mask >> k) & 1u) { Ext v; for (int i = 0; i < 10; ++i) v.b[i] = r.st_xout[k][i]; return v; }
    return ext(r.st[k]);
}
inline void set_in_st(Regs& r, int k, const Ext& v) {
    for (int i = 0; i < 10; ++i) r.st_xin[k][i] = v.b[i];
    r.st_xin_mask |= 1u << k;
    r.st[k] = narrow(v);
}

// Rounding helpers for when the original stores to float/double memory in the
// middle of a calculation (forces the same precision loss).
inline float  to_f32(double x) { volatile float f = static_cast<float>(x); return f; }
inline double to_f64(double x) { volatile double d = x; return d; }

}}  // namespace gp4::x87
