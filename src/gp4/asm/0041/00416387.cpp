#include <gp4/gp4.h>

// FUNCTION: GP4 0x00416387
// Unless byte [esi+0x7c] is negative AND bit 8 of byte [esi+0x78] is set, marks the record
// (bit 0x10 of the bytes at +0xcc and +0xce) and runs the record update at 0x418097.
void __cdecl FUN_00416387(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t flagsA = GP4_FIELD(uint8_t, record, 0x7c);
    const uint8_t flagsB = GP4_FIELD(uint8_t, record, 0x78);
    if ((flagsA & 0x80u) != 0u && (flagsB & 8u) != 0u) {
        // flags of `test byte [esi+0x78], 8` with result 8: CF=OF=AF=0, PF=0, ZF=0, SF=0
        r->eflags = r->eflags & ~0x8d5u;
        return;
    }
    GP4_FIELD(uint8_t, record, 0xcc) |= 0x10u;
    GP4_FIELD(uint8_t, record, 0xce) |= 0x10u;
    gp4::Regs call = *r;
    gp4::call_regs(0x00418097, call);
    r->eflags = call.eflags;
}
GP4_IMPL(0x00416387, FUN_00416387, "regs(esi:ptr[0x106]:bytes) -> (cf, pf, af, zf, sf, of)")
