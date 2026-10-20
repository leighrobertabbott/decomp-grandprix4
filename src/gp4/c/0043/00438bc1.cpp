#include <gp4/gp4.h>

// Runs 0x00438aac on the record in esi, then 0x00413d01 when bit 2 of the byte at +0x203 is set. The flags
// are those of 0x00413d01, or of the `test byte [esi+0x203], 4` (PF and ZF set when the bit is clear).
// FUNCTION: GP4 0x00438bc1
void __cdecl FUN_00438bc1(gp4::Regs* r) {
    gp4::Regs call = *r;
    gp4::call_regs(0x00438aac, call);
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(call.esi));
    if ((record[0x203] & 4u) != 0u) {
        gp4::call_regs(0x00413d01, call);
    } else {
        call.eflags = (call.eflags & ~0x8d5u) | 0x44u;
    }
    *r = call;
}
GP4_IMPL(0x00438bc1, FUN_00438bc1, "regs(esi:ptr[0x35c]:layout=rec_00413d01) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_00413d01")
