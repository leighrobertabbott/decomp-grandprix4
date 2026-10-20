#include <gp4/gp4.h>

// Loads the segment pointer at +0x1c of the record in esi into edi, lets 0x00405b8c walk it along the
// 0x160-byte segment ring, and stores the walked pointer (edi after the call) back at +0x1c.
// FUNCTION: GP4 0x00405a06
void __cdecl FUN_00405a06(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs call = *r;
    call.edi = GP4_FIELD(uint32_t, record, 0x1c);
    gp4::call_regs(0x00405b8c, call);
    GP4_FIELD(uint32_t, record, 0x1c) = call.edi;
    *r = call;
}
GP4_IMPL(0x00405a06, FUN_00405a06, "regs(esi:u32[0x66fef0..0x66fef0]) -> (eax, ebx, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_004317ae")
