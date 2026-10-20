#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a dword-wide logic result (or [mem], imm): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// Settles the position (eax, edx) from the segment record kept at 0x00627938 (0x00406ebc leaves the
// new record in 0x00629f0c and the lateral step in 0x00629f04), stores the new record back into
// 0x00627938, then measures how far the caller's record edi lies from it in records (0x0040cf8c:
// quotient of the byte distance by 0x160) and ORs 1 (same), 4 (negative) or 2 (positive) into the
// state word 0x0062793c. eax = 0x00629f04; ecx and edx are those left by the two calls; edi is
// preserved; the flags are those of the final `or [0x0062793c], n`.
// FUNCTION: GP4 0x0040cfc9
void __cdecl FUN_0040cfc9(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.edi = GP4_GLOBAL(uint32_t, 0x00627938);
    gp4::call_regs(0x00406ebc, call);

    const uint32_t record = GP4_GLOBAL(uint32_t, 0x00629f0c);
    GP4_GLOBAL(uint32_t, 0x00627938) = record;

    // pop edi restores the caller's edi; eax = that edi, edx = the new record
    call.eax = r->edi;
    call.edx = record;
    gp4::call_regs(0x0040cf8c, call);

    uint32_t bit;
    if (call.eax == 0u) {
        bit = 1u;
    } else if (static_cast<int32_t>(call.eax) < 0) {
        bit = 4u;
    } else {
        bit = 2u;
    }
    const uint32_t state = GP4_GLOBAL(uint32_t, 0x0062793c) | bit;
    GP4_GLOBAL(uint32_t, 0x0062793c) = state;

    r->eax = GP4_GLOBAL(uint32_t, 0x00629f04);
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(state);
}
GP4_IMPL(0x0040cfc9, FUN_0040cfc9, "regs(eax:u32, edx:u32, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+ptr_00627938+scan_00406d4b+lap_0040cf8c")
