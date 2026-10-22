#include <gp4/gp4.h>

// Runs the shared list-teardown routine 0x005bb100 on the head cell 0x00b348f8
// (`mov ecx, 0xb348f8; jmp 0x5bb100`).
// FUNCTION: GP4 0x005b5470
void __cdecl FUN_005b5470() {
    gp4::thiscall_<0x005bb100, void>((void*)0x00b348f8);
}
GP4_IMPL(0x005b5470, FUN_005b5470, "cdecl() -> void")
