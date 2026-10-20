#include <gp4/gp4.h>

// Scans the 22 car records (0x41c bytes each, from 0x0066fef0) for the first whose byte +0x415 is
// zero. esi = that record (found: `and ecx,0` leaves ZF = PF = 1) or the first record when every
// byte is nonzero (`or ecx,-1` leaves SF = PF = 1, ZF = 0). ecx is saved and restored.
// FUNCTION: GP4 0x00409aec
void __cdecl FUN_00409aec(gp4::Regs* r) {
    uint8_t* record = GP4_ARRAY(uint8_t, 0x0066fef0);
    for (int remaining = 0x16; remaining != 0; --remaining) {
        if (GP4_FIELD(uint8_t, record, 0x415) == 0) {
            r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
            r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
            return;
        }
        record += 0x41c;
    }
    r->esi = 0x0066fef0u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x84u;
}
GP4_IMPL(0x00409aec, FUN_00409aec, "regs() -> (esi, cf, pf, af, zf, sf, of); globals=layout=scan_00409aec")
