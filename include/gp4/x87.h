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

// Rounding helpers for when the original stores to float/double memory in the
// middle of a calculation (forces the same precision loss).
inline float  to_f32(double x) { volatile float f = static_cast<float>(x); return f; }
inline double to_f64(double x) { volatile double d = x; return d; }

}}  // namespace gp4::x87
