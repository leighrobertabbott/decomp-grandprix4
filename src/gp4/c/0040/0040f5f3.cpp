#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040f5f3
// Walks the 22 records (stride 0x41c) at 0x0066fef0 and runs 0x0040f458 on every record whose byte
// +0x7a has neither bit 7 nor bit 6 set (esi = the record, ecx = the remaining count), then calls
// 0x0040f898 with esi one past the last record and ecx = 0. Everything it leaves is what that last
// call leaves.
void __cdecl FUN_0040f5f3(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    uint32_t record = 0x0066fef0u;
    for (uint32_t remaining = 0x16u; remaining != 0u; --remaining) {
        const uint8_t flags7a = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(record + 0x7au));
        if ((flags7a & 0x80u) == 0u && (flags7a & 0x40u) == 0u) {
            call.esi = record;
            call.ecx = remaining;
            gp4::call_regs(0x0040f458, call);
        }
        record += 0x41cu;
    }

    call.esi = record;
    call.ecx = 0u;
    gp4::call_regs(0x0040f898, call);

    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->esi = call.esi;
    r->edi = call.edi;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0040f5f3, FUN_0040f5f3, "regs(eax:u32, edi:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=state_0040f5f3")
