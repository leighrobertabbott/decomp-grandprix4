#include <gp4/gp4.h>

// atexit destructor of the static pool at 0x00a52030: runs the shared pool-teardown routine
// 0x004ac390 on it (`mov ecx, 0xa52030; jmp 0x4ac390`), which frees the block list.
// FUNCTION: GP4 0x00493940
void __cdecl FUN_00493940() {
    gp4::thiscall_<0x004ac390, void>((void*)0x00a52030);
}
GP4_IMPL(0x00493940, FUN_00493940, "cdecl() -> void")
