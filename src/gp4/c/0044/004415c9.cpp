#include <gp4/gp4.h>

// pushal wrapper for the car record in esi: runs 0x0044182d, stores the word it leaves in cx at +0xa2 of the record,
// then runs 0x004413b7. popal restores every register; the flags are those 0x004413b7 leaves.
// FUNCTION: GP4 0x004415c9
void __cdecl FUN_004415c9(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0044182d, call);
    GP4_FIELD(uint16_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(call.esi)), 0xa2) =
        static_cast<uint16_t>(call.ecx);
    gp4::call_regs(0x004413b7, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x004415c9, FUN_004415c9, "regs(esi:ptr[0x41c]:layout=rec_0043259c) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043c1ba+state_004320f2+counter_0040f0dd+tabs_004413b7+scale_0066ec60")
