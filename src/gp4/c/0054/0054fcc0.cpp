#include <gp4/gp4.h>

// `call 0x0054fe9b ; call 0x00549d81 ; ret`: draws the text box record esi (queues it in the sorted
// list twice, fills and labels it: 0x0054fe9b) and then rebuilds the list's node pool (0x00549d81).
// The registers and flags each call leaves carry into the next; everything comes back from the
// second. edx is an input of the colour/text-cursor helpers the first call reaches.
// FUNCTION: GP4 0x0054fcc0
void __cdecl FUN_0054fcc0(gp4::Regs* r) {
    gp4::call_regs(0x0054fe9b, *r);
    gp4::call_regs(0x00549d81, *r);
}
GP4_IMPL(0x0054fcc0, FUN_0054fcc0, "regs(edx:u32, esi:ptr[0x10]:u32[0..7]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+pool_00549db9+script_0054d798+textout_0054d211")
