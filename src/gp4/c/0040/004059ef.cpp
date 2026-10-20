#include <gp4/gp4.h>

// Follows the link of the segment record in edi (0x004059d2), then updates the car record in esi with
// 0x00405e6e when flag 0x20 of the record's byte +0x1b is set, with 0x00405efa when it is clear. Both
// routines leave the registers and flags the caller sees.
// FUNCTION: GP4 0x004059ef
void __cdecl FUN_004059ef(gp4::Regs* r) {
    gp4::Regs call = *r;
    gp4::call_regs(0x004059d2, call);
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(call.edi));
    if ((record[0x1b] & 0x20u) != 0u) {
        gp4::call_regs(0x00405e6e, call);
        *r = call;
        return;
    }
    gp4::call_regs(0x00405efa, call);
    *r = call;
}
GP4_IMPL(0x004059ef, FUN_004059ef, "regs(esi:ptr[0x420]:bytes, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef")
