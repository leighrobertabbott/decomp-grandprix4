#include <gp4/gp4.h>

// Steps the record pointer in edi along its link (0x004059d2; CF set = no link), then re-walks it with the
// point at esi through 0x00405d65 (flag 0x20 of byte +0x1b set) or 0x00405cf8 (clear). CF is set when the
// walk changed flag 0x20 of the record's byte +0x1b, clear when it kept it; the other flags are those of
// the final `test` (or of 0x004059d2 when it reports no link).
// FUNCTION: GP4 0x00406123
void __cdecl FUN_00406123(gp4::Regs* r) {
    gp4::Regs call = *r;
    gp4::call_regs(0x004059d2, call);
    if ((call.eflags & 1u) != 0u) {
        // jb: stc keeps the other flags of 0x004059d2
        *r = call;
        return;
    }
    const uint8_t* before = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(call.edi));
    if ((before[0x1b] & 0x20u) != 0u) {
        gp4::call_regs(0x00405d65, call);
        const uint8_t* after = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(call.edi));
        const bool cleared = (after[0x1b] & 0x20u) == 0u;
        // test byte [edi+0x1b], 0x20 then je -> stc (cleared) or clc (still set)
        call.eflags = (call.eflags & ~0x8d5u) | (cleared ? 0x45u : 0u);
    } else {
        gp4::call_regs(0x00405cf8, call);
        const uint8_t* after = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(call.edi));
        const bool raised = (after[0x1b] & 0x20u) != 0u;
        // test byte [edi+0x1b], 0x20 then jne -> stc (raised) or clc (still clear)
        call.eflags = (call.eflags & ~0x8d5u) | (raised ? 1u : 0x44u);
    }
    *r = call;
}
GP4_IMPL(0x00406123, FUN_00406123, "regs(esi:ptr[0x40]:layout=pos_00405cf8, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12+links_00405a12")
