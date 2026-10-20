// gp4.h - conventions every reconstruction uses.
//
//   // FUNCTION: GP4 0x00401d80
//   void __cdecl Body_ComputeAngleSinCos(gp4::Regs* r) { ... }
//   GP4_IMPL(0x00401d80, Body_ComputeAngleSinCos, "regs(edx:ptr[0x60]:f64) -> ()")
//
// * GP4_IMPL registers the function in the .gp4reg section. The emulator oracle
//   (gp4re verify) and the in-game hook DLL both discover reconstructions there.
// * Unreconstructed GP4 functions are called through their original address
//   (gp4::cdecl_<addr, R, Args...>), globals are accessed with GP4_GLOBAL. That
//   keeps every reconstruction runnable against the real binary from day one.
// * thiscall methods are written as __fastcall free functions:
//       int __fastcall CCar_GetGear(CCar* self, void* /*edx*/, int arg)
//   which is ABI-identical to MSVC thiscall.
#pragma once
#include <stdint.h>
#include <stddef.h>

#define GP4_CONCAT_(a, b) a##b
#define GP4_CONCAT(a, b) GP4_CONCAT_(a, b)

// ---------------------------------------------------------------- registry
#pragma section(".gp4reg$a", read)
#pragma section(".gp4reg$m", read)
#pragma section(".gp4reg$z", read)

namespace gp4 {
struct ImplRecord {
    uint32_t addr;        // original GP4 address
    const void* impl;     // reconstruction entry point
    const char* name;     // function name
    const char* spec;     // calling convention + test-input spec (see gp4re/spec.py)
};
static_assert(sizeof(ImplRecord) == 16, "x86 only");

// Register file handed to reconstructions of hand-written assembly routines
// ("regs" convention). Inputs are pre-loaded, outputs are written back.
struct Regs {
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp;
    uint32_t eflags;
    double st[8];     // st[0] = top of the x87 stack on entry / exit
    int32_t st_in;    // number of x87 inputs supplied
    int32_t st_out;   // number of x87 outputs expected back
    // 80-bit x87 values (hand-written asm passes extended precision on the stack).
    // st_xin[k] is valid when bit k of st_xin_mask is set (filled by the caller /
    // adapter); a producer of outputs sets st_xout[k] and bit k of st_xout_mask,
    // otherwise the double st[k] is the output. Use gp4::x87::in_st/set_out_st.
    uint8_t st_xin[8][10];
    uint8_t st_xout[8][10];
    uint32_t st_xin_mask;
    uint32_t st_xout_mask;
};
static_assert(sizeof(Regs) == 0x110, "layout shared with gp4re/emu.py and the hook adapters");

enum : uint32_t { CF = 1u << 0, ZF = 1u << 6, SF = 1u << 7 };
}  // namespace gp4

#define GP4_IMPL(addr, fn, spec)                                                         \
    extern "C" __declspec(allocate(".gp4reg$m")) const gp4::ImplRecord                    \
        GP4_CONCAT(gp4_impl_, addr) = {addr, (const void*)&fn, #fn, spec};

// ---------------------------------------------------------------- original memory
#define GP4_GLOBAL(type, addr) (*reinterpret_cast<type*>(static_cast<uintptr_t>(addr)))
#define GP4_ARRAY(type, addr) (reinterpret_cast<type*>(static_cast<uintptr_t>(addr)))
// Call a Win32 import through GP4's own IAT slot (identical behaviour in-game,
// trapped as "external" by the emulator).
#define GP4_IAT(type, slot) (*reinterpret_cast<type*>(static_cast<uintptr_t>(slot)))

// Byte offset field access for structures whose layout is still being recovered:
//   GP4_FIELD(float, car, 0x2c) = 0.0f;
#define GP4_FIELD(type, base, off) \
    (*reinterpret_cast<type*>(reinterpret_cast<uint8_t*>(base) + (off)))

// ---------------------------------------------------------------- 64/32 division
// The freestanding runtime has no __alldiv/__aulldiv, so int64 '/' does not link.
// These MSVC intrinsics compile to the single IDIV/DIV instruction the original
// executes: quotient and remainder of edx:eax by a 32-bit divisor. As with the
// instruction, the quotient must fit 32 bits (the original faults otherwise).
extern "C" int __cdecl _div64(__int64 dividend, int divisor, int* remainder);
extern "C" unsigned int __cdecl _udiv64(unsigned __int64 dividend, unsigned int divisor, unsigned int* remainder);
namespace gp4 {
inline int32_t idiv64(int64_t dividend, int32_t divisor, int32_t* remainder) {
    int rem; const int q = _div64(dividend, divisor, &rem); *remainder = rem; return q;
}
inline uint32_t div64(uint64_t dividend, uint32_t divisor, uint32_t* remainder) {
    unsigned int rem; const unsigned int q = _udiv64(dividend, divisor, &rem); *remainder = rem; return q;
}
}  // namespace gp4

// ---------------------------------------------------------------- calling originals
namespace gp4 {
template <uintptr_t A, typename R, typename... Args>
inline R cdecl_(Args... a) { return reinterpret_cast<R(__cdecl*)(Args...)>(A)(a...); }

template <uintptr_t A, typename R, typename... Args>
inline R stdcall_(Args... a) { return reinterpret_cast<R(__stdcall*)(Args...)>(A)(a...); }

template <uintptr_t A, typename R, typename Self, typename... Args>
inline R thiscall_(Self* self, Args... a) {
    return reinterpret_cast<R(__fastcall*)(Self*, void*, Args...)>(A)(self, nullptr, a...);
}

// Call an original hand-written-assembly routine with an explicit register file.
// Implemented in src/gp4/runtime/gp4_rt.cpp. Supports GPR in/out and up to 8 x87
// values in/out (r.st_in / r.st_out).
extern "C" void __cdecl gp4_call_regs(uint32_t addr, Regs* r);
inline void call_regs(uintptr_t addr, Regs& r) { gp4_call_regs(static_cast<uint32_t>(addr), &r); }

// Same for a stdcall routine called with live registers: GPRs and EFLAGS from r, the dwords of
// args pushed as stack arguments (args[0] first), the callee pops them ('ret 4*N'); GPRs and
// EFLAGS come back in r (no x87). E.g. eax = selector; push [g]; call 0x546636 keeps eax intact
// on a path where the callee does not touch it.
extern "C" void __cdecl gp4_call_regs_stdcall(uint32_t addr, Regs* r, const uint32_t* args, uint32_t n);
template <int N>
inline void call_regs_stdcall(uintptr_t addr, Regs& r, const uint32_t (&args)[N]) {
    gp4_call_regs_stdcall(static_cast<uint32_t>(addr), &r, args, static_cast<uint32_t>(N));
}
}  // namespace gp4
