#include <gp4/gp4.h>

// FUNCTION: GP4 0x004112dd
void __cdecl FUN_004112dd(gp4::Regs* r) {
    gp4::call_regs(0x00410dd2, *r);
}
GP4_IMPL(0x004112dd, FUN_004112dd, "regs(ebx:ptr[0x26]:bytes, esi:ptr[0x422]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=flag_007a97a0; cases=200")
