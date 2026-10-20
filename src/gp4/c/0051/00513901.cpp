#include <gp4/gp4.h>

// Copies four dwords of the table at 0x00679ba4.. to 0x0088f614..: 0x679ba4 -> 0x88f614,
// 0x679bb4 -> 0x88f618, 0x679ba8 -> 0x88f61c, 0x679bb8 -> 0x88f620. eax ends as the last value
// copied; no flag is touched.
// FUNCTION: GP4 0x00513901
void __cdecl FUN_00513901(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0088f614) = GP4_GLOBAL(uint32_t, 0x00679ba4);
    GP4_GLOBAL(uint32_t, 0x0088f618) = GP4_GLOBAL(uint32_t, 0x00679bb4);
    GP4_GLOBAL(uint32_t, 0x0088f61c) = GP4_GLOBAL(uint32_t, 0x00679ba8);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00679bb8);
    GP4_GLOBAL(uint32_t, 0x0088f620) = last;
    r->eax = last;
}
GP4_IMPL(0x00513901, FUN_00513901, "regs() -> (eax); globals=fuzz:i32")
