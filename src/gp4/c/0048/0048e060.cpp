#include <gp4/gp4.h>

// Calls the D3DX thunk at 0x0055c3ee (stdcall, three pointers) with `out` and the two 0x40-byte records at
// self + index * 0x40 + 0x4368 and self + 0x334; returns its result.
// FUNCTION: GP4 0x0048e060
int __fastcall FUN_0048e060(uint8_t* self, void* /*edx*/, int index, void* out) {
    return gp4::stdcall_<0x0055c3ee, int>(out, self + index * 0x40 + 0x4368, self + 0x334);
}
GP4_IMPL(0x0048e060, FUN_0048e060, "thiscall(ptr[0x4800]:f32, i32[0..7], ptr[0x40]:f32) -> i32")
