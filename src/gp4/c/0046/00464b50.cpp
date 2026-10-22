#include <gp4/gp4.h>

typedef int(__stdcall* Fn_00560817)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                                    uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

// Forwards all fifteen of its stack arguments, unchanged and in order, to 0x00560817 (a library
// routine that pops its own arguments, ret 0x3c), and returns that routine's result.
// FUNCTION: GP4 0x00464b50
int __cdecl FUN_00464b50(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7,
                         uint32_t a8, uint32_t a9, uint32_t a10, uint32_t a11, uint32_t a12, uint32_t a13,
                         uint32_t a14, uint32_t a15) {
    return reinterpret_cast<Fn_00560817>(0x00560817)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}
GP4_IMPL(0x00464b50, FUN_00464b50, "cdecl(nptr[16]:com=IDirect3DDevice8, nptr[0x100]:bytes, u32[0..0x100], u32, u32, u32, u32, u32, u32, u32, u32, u32, nptr[0x40]:bytes, nptr[0x10]:bytes, nptr[0x10]:bytes) -> i32")
