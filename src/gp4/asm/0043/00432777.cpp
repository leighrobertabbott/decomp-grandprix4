#include <gp4/gp4.h>

// FUNCTION: GP4 0x00432777
// Clears bit 0x20 of byte [esi+0x25f], then runs 0x434044 with eax = 4 (when byte [0x007c2ea9]
// is 0x80) or 2. pushal/popal restore every register; the flags come from the callee.
void __cdecl FUN_00432777(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_FIELD(uint8_t, record, 0x25f) &= 0xdfu;
    gp4::Regs call = *r;
    call.eax = (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) ? 4u : 2u;
    gp4::call_regs(0x00434044, call);
    r->eflags = call.eflags;
}
GP4_IMPL(0x00432777, FUN_00432777, "regs(esi:ptr[0x267]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
