#include <gp4/gp4.h>

// Loads the point (ecx, ebx) from the first two dwords of the record in esi and lets 0x00405c9a walk
// the record pointer in edi to the segment containing it; ebx is saved and restored around the call.
// FUNCTION: GP4 0x00405cf8
void __cdecl FUN_00405cf8(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t savedEbx = r->ebx;
    gp4::Regs call = *r;
    call.ecx = GP4_FIELD(uint32_t, record, 0);
    call.ebx = GP4_FIELD(uint32_t, record, 4);
    gp4::call_regs(0x00405c9a, call);
    call.ebx = savedEbx;
    *r = call;
}
GP4_IMPL(0x00405cf8, FUN_00405cf8, "regs(esi:ptr[0x40]:bytes, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
