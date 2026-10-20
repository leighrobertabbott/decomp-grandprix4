#include <gp4/gp4.h>

// Inverse of 0x0041d5a1: copies the three dwords DAT_00602fcc/fd0/fd4 to DAT_00622be8/bec/bf0,
// runs the body block at 0x00622e98 through 0x00402290 (integers -> doubles at +0xb8..),
// 0x00401f02 (matrix rotation into +0xa0..) and 0x004022b5 (rounds +0xa0.. back to the integers
// DAT_00622bdc/be0/be4) and copies those three results to DAT_00623eb8/ebc/ec0. Leaves
// edx = 0x00622e98 and eax = the last copied value; the x87 stack is empty.
// FUNCTION: GP4 0x0041d5f3
void __cdecl FUN_0041d5f3(gp4::Regs* r) {
    GP4_GLOBAL(int32_t, 0x00622be8) = GP4_GLOBAL(int32_t, 0x00602fcc);
    GP4_GLOBAL(int32_t, 0x00622bec) = GP4_GLOBAL(int32_t, 0x00602fd0);
    GP4_GLOBAL(int32_t, 0x00622bf0) = GP4_GLOBAL(int32_t, 0x00602fd4);

    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00622e98)));
    gp4::call_regs(0x00402290, *r);
    gp4::call_regs(0x00401f02, *r);
    gp4::call_regs(0x004022b5, *r);

    GP4_GLOBAL(int32_t, 0x00623eb8) = GP4_GLOBAL(int32_t, 0x00622bdc);
    GP4_GLOBAL(int32_t, 0x00623ebc) = GP4_GLOBAL(int32_t, 0x00622be0);
    const int32_t last = GP4_GLOBAL(int32_t, 0x00622be4);
    GP4_GLOBAL(int32_t, 0x00623ec0) = last;
    r->eax = static_cast<uint32_t>(last);
}
GP4_IMPL(0x0041d5f3, FUN_0041d5f3, "regs() -> (eax, edx); globals=fuzz:i32+layout=view_00609860")
