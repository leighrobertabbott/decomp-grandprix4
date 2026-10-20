#include <gp4/gp4.h>

// Tail wrapper: "mov ax, 0x4b; call 0x00531f9c; ret". The upper half of eax is untouched; the
// dispatcher's eax and flags are the result. Command 0x4b runs handler 0x00532d7f -> 0x0054b8fe, which
// stores the packet [0x7a97ac] bytes long in the send slot and reads it back.
// FUNCTION: GP4 0x0051f0ae
void __cdecl FUN_0051f0ae(gp4::Regs* r) {
    r->eax = (r->eax & 0xffff0000u) | 0x4bu;
    gp4::call_regs(0x00531f9c, *r);
}
GP4_IMPL(0x0051f0ae, FUN_0051f0ae, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0054b978")
