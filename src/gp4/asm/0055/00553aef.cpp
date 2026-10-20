#include <gp4/gp4.h>
#include <gp4/flags.h>

// Vector from 0x00553a5e with the length -(word at [edi+0x60]) << 8, stored in 0x934df0/0x934df4
// (eax -> 0x934df0, edx -> 0x934df4). eax/edx and the flags leave as 0x00553a5e left them.
// FUNCTION: GP4 0x00553aef
void __cdecl FUN_00553aef(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    r->eax = (0u - static_cast<uint32_t>(GP4_FIELD(uint16_t, record, 0x60))) << 8;
    gp4::call_regs(0x00553a5e, *r);
    GP4_GLOBAL(uint32_t, 0x00934df0) = r->eax;
    GP4_GLOBAL(uint32_t, 0x00934df4) = r->edx;
}
GP4_IMPL(0x00553aef, FUN_00553aef, "regs(edi:ptr[0x100]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:f64")
