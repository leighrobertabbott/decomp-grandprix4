#include <gp4/gp4.h>

// Walks the 64 slots: for every active one (byte flags at 0x0063bcfc) whose dword +0x4c in the
// 0x78-byte record array at 0x0063bd7c differs from the remembered value in 0x006098c0, remembers
// it through 0x0042453d and refreshes the record through 0x00424553 (both keep every register).
// eax = the last dword loaded, ecx = 0x40, esi = one past the last record; the flags are those of
// the final `cmp ecx, 0x40` (equal).
// FUN_0042506e has no inputs but eax (only read when no slot is active).
// FUNCTION: GP4 0x0042506e
void __cdecl FUN_0042506e(gp4::Regs* r) {
    const uint8_t* active = GP4_ARRAY(const uint8_t, 0x0063bcfc);
    const uint32_t* remembered = GP4_ARRAY(const uint32_t, 0x006098c0);
    uint32_t record = 0x0063bd7cu;

    for (uint32_t slot = 0; slot != 0x40u; ++slot, record += 0x78u) {
        if (active[slot] != 0u) {
            r->eax = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(record) + 0x4cu);
            if (r->eax != remembered[slot]) {
                r->ecx = slot;
                r->esi = record;
                gp4::call_regs(0x0042453d, *r);
                gp4::call_regs(0x00424553, *r);
            }
        }
    }

    r->ecx = 0x40u;
    r->esi = record;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0042506e, FUN_0042506e, "regs(eax:u32) -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=layout=state_0042458b+recs_0063bd7c")
