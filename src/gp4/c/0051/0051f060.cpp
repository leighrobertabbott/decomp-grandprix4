#include <gp4/gp4.h>

// Tail wrapper: `call 0x004814c0 ; ret`. The callee clears the six flag bytes of the 0x218-byte record
// selected by the dword 0x00a4df50 (record array at 0x00a4b430) and sets the first byte of the record
// picked by the dword 0x00645950 - 1 when the byte 0x007c2e6c is set (record 0 otherwise). eax, ecx,
// edx and the flags come straight back from the callee.
// FUNCTION: GP4 0x0051f060
void __cdecl FUN_0051f060(gp4::Regs* r) {
    gp4::call_regs(0x004814c0, *r);
}
GP4_IMPL(0x0051f060, FUN_0051f060, "regs() -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00a4df50")
