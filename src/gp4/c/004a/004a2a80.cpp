#include <gp4/gp4.h>

// Looks up the dword at 0x0064a908 indexed by a signed byte argument.
// FUNCTION: GP4 0x004a2a80
int __cdecl FUN_004a2a80(int8_t index) { return GP4_ARRAY(int32_t, 0x0064a908)[index]; }
GP4_IMPL(0x004a2a80, FUN_004a2a80, "cdecl(i8) -> i32")
