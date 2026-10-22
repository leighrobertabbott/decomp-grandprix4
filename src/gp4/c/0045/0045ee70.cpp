#include <gp4/gp4.h>

// Returns element `index` of the dword array at 0x009fb3f8.
// FUNCTION: GP4 0x0045ee70
uint32_t __cdecl FUN_0045ee70(int index) { return GP4_ARRAY(uint32_t, 0x009fb3f8)[index]; }
GP4_IMPL(0x0045ee70, FUN_0045ee70, "cdecl(i32[0..15]) -> u32; globals=fuzz:i32")
