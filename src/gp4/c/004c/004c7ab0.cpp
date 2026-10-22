#include <gp4/gp4.h>

#pragma pack(push, 1)
// Three consecutive dwords at 0x00a71848 (copied as integers, a vector of unrecovered type).
struct Triple { uint32_t A, B, C; };
#pragma pack(pop)

// Copies the three dwords 0x00a71848/4c/50 into the caller's buffer (a struct returned by value through the
// hidden pointer) and returns that pointer.
// FUNCTION: GP4 0x004c7ab0
Triple* __stdcall FUN_004c7ab0(Triple* out) {
    Triple value;
    value.A = GP4_GLOBAL(uint32_t, 0x00a71848);
    value.B = GP4_GLOBAL(uint32_t, 0x00a7184c);
    value.C = GP4_GLOBAL(uint32_t, 0x00a71850);
    *out = value;
    return out;
}
GP4_IMPL(0x004c7ab0, FUN_004c7ab0, "stdcall(ptr[0x10]:bytes) -> u32; globals=fuzz:i32")
