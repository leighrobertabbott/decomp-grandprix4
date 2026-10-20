#include <gp4/gp4.h>

// Copies the three dwords DAT_00623eb8/ebc/ec0 to DAT_00622bdc/be0/be4, runs the body block at
// 0x00622e98 through 0x00402246 (integers -> doubles at +0xa0..), 0x00401e2e (matrix multiply into
// +0xb8..) and 0x0040226b (rounds +0xb8.. back to the integers DAT_00622be8/bec/bf0) and copies
// those three results to DAT_00602fcc/fd0/fd4. Leaves edx = 0x00622e98 and eax = the last copied
// value; the x87 stack is empty.
// FUNCTION: GP4 0x0041d5a1
void __cdecl FUN_0041d5a1(gp4::Regs* r) {
    GP4_GLOBAL(int32_t, 0x00622bdc) = GP4_GLOBAL(int32_t, 0x00623eb8);
    GP4_GLOBAL(int32_t, 0x00622be0) = GP4_GLOBAL(int32_t, 0x00623ebc);
    GP4_GLOBAL(int32_t, 0x00622be4) = GP4_GLOBAL(int32_t, 0x00623ec0);

    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00622e98)));
    gp4::call_regs(0x00402246, *r);
    gp4::call_regs(0x00401e2e, *r);
    gp4::call_regs(0x0040226b, *r);

    GP4_GLOBAL(int32_t, 0x00602fcc) = GP4_GLOBAL(int32_t, 0x00622be8);
    GP4_GLOBAL(int32_t, 0x00602fd0) = GP4_GLOBAL(int32_t, 0x00622bec);
    const int32_t last = GP4_GLOBAL(int32_t, 0x00622bf0);
    GP4_GLOBAL(int32_t, 0x00602fd4) = last;
    r->eax = static_cast<uint32_t>(last);
}
GP4_IMPL(0x0041d5a1, FUN_0041d5a1, "regs() -> (eax, edx); globals=fuzz:i32+layout=view_00609860")
