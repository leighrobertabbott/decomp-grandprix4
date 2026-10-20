#include <gp4/gp4.h>

// FUNCTION: GP4 0x004091ef
// Initialises the 22 car records of the 0x41c-stride array at 0x0066fef0: for record i it calls
// 0x00409213 with ebx = i, esi = the record and al = the key byte [0x0066d8f4 + i]. The pushal/popal
// pair preserves every GPR; the flags are those of the last "cmp ebx, 0x16" (equal).
void __cdecl FUN_004091ef(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    const uint8_t* keys = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(0x0066d8f4u));
    const uint32_t upperEax = r->eax & 0xffffff00u;
    uint32_t record = 0x0066fef0u;
    for (uint32_t index = 0; index != 0x16u; ++index) {
        call.ebx = index;
        call.esi = record;
        call.eax = upperEax | keys[index];
        gp4::call_regs(0x00409213, call);
        record += 0x41cu;
    }

    // last flag writer: cmp ebx, 0x16 with ebx equal -> ZF = PF = 1
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004091ef, FUN_004091ef, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
