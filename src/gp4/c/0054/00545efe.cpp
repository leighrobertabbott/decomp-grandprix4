#include <gp4/gp4.h>

// `call 0x0054d1ad ; call 0x00545f09 ; ret`: esi = table[esi] (the string pointer for text entry esi), then the
// text-script interpreter 0x00545f09 walks that string. Registers and flags come straight back from the second call.
// FUNCTION: GP4 0x00545efe
void __cdecl FUN_00545efe(gp4::Regs* r) {
    gp4::call_regs(0x0054d1ad, *r);
    gp4::call_regs(0x00545f09, *r);
}
GP4_IMPL(0x00545efe, FUN_00545efe, "regs(ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:u32[0..7]) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cursor_00545f09+surf_0054654a+script_00545efe")
