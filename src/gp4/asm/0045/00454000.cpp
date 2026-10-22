#include <gp4/gp4.h>

// Stub taking three stack arguments (ret 0xc) that returns -1; the caller
// compares eax with -1 straight after the call.
// FUNCTION: GP4 0x00454000
int __stdcall FUN_00454000(int, int, int) { return -1; }
GP4_IMPL(0x00454000, FUN_00454000, "stdcall(i32, i32, i32) -> i32")
