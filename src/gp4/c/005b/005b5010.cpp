#include <gp4/gp4.h>

// Tail jump to the empty routine 0x005b5020 (a single `ret`): `jmp 0x5b5020`.
// FUNCTION: GP4 0x005b5010
void __cdecl FUN_005b5010() { gp4::cdecl_<0x005b5020, void>(); }
GP4_IMPL(0x005b5010, FUN_005b5010, "cdecl() -> void")
