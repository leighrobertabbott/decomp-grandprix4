#include <gp4/gp4.h>

// Releases the heap block whose pointer is stored at this+0 (FUN_005dc114 is the CRT free).
// FUNCTION: GP4 0x00481710
void __fastcall FUN_00481710(void** self, void* /*edx*/) {
    gp4::cdecl_<0x005dc114, void>(*self);
}
GP4_IMPL(0x00481710, FUN_00481710, "thiscall(ptr[0x10]:bytes) -> void")
