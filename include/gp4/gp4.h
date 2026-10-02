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
};
static_assert(sizeof(Regs) == 0x68, "layout shared with gp4re/emu.py and the hook adapters");

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
}  // namespace gp4
