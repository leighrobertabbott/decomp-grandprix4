#include <gp4/gp4.h>

// Stub returning the constant 1 (mov eax, 1).
// FUNCTION: GP4 0x004aa530
int __cdecl FUN_004aa530() { return 1; }
GP4_IMPL(0x004aa530, FUN_004aa530, "cdecl() -> i32")
