#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// After the callee: CF = 0 -> clc with the callee's other flags and eax = the slot; CF = 1 ->
// the slot is doubled (shl 1; AF = 0, OF = bit31 ^ CF), then stc.
static __forceinline void finish(gp4::Regs* r, uint32_t* slots, uint32_t slot, uint32_t eax,
                                 uint32_t calleeFlags) {
    if ((calleeFlags & 1u) == 0u) {
        r->eax = eax;
        r->eflags = (r->eflags & ~0x8d5u) | (calleeFlags & 0x8d4u);
        return;
    }

    eax = slots[slot];
    const uint32_t doubled = eax << 1;
    slots[slot] = doubled;
    const uint32_t flags = 0x01u
        | parityFlag8(doubled)
        | (doubled == 0u ? 0x40u : 0u)
        | ((doubled & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((eax ^ doubled) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eax = doubled;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// FUNCTION: GP4 0x004067b7
// Tries the perpendicular foot of slot ebx (dword at 0x00629f04[ebx]): a zero slot fails at once;
// a negative one asks 0x004065c3, a positive one 0x004066be. On success (CF = 1 from the callee)
// the slot is doubled (shl 1) and CF = 1 is returned with eax = the doubled value; otherwise
// CF = 0 with the flags the callee (or `or eax, eax` for a zero slot) left and eax = the slot.
// All registers but eax are preserved.
void __cdecl FUN_004067b7(gp4::Regs* r) {
    uint32_t* slots = GP4_ARRAY(uint32_t, 0x00629f04);
    const uint32_t slot = r->ebx;
    const uint32_t eax = slots[slot];

    if (eax == 0u) {
        // or eax, eax -> ZF, PF set; clc
        r->eax = 0u;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = eax;
    if (static_cast<int32_t>(eax) < 0) {
        gp4::call_regs(0x004065c3, call);
        finish(r, slots, slot, eax, call.eflags);
    } else {
        gp4::call_regs(0x004066be, call);
        finish(r, slots, slot, eax, call.eflags);
    }
}
GP4_IMPL(0x004067b7, FUN_004067b7, "regs(edi:ptr[0x136]:bytes, ebx:u32[0..0]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004067ea")
