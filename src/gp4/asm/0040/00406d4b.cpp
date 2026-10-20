#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// The ring search used when the byte 0x00627948 is nonzero (walks forward).
static __declspec(noinline) void searchForward(gp4::Regs& call) {
    gp4::call_regs(0x00406b6c, call);
}

// The ring search used when the byte 0x00627948 is zero (walks backward).
static __declspec(noinline) void searchBackward(gp4::Regs& call) {
    gp4::call_regs(0x00406c6d, call);
}

// FUNCTION: GP4 0x00406d4b
// Remembers the search start (edi, eax) in 0x006278e8 / 0x006278ec, runs the ring search 0x00406b6c
// (when the byte 0x00627948 is nonzero) or 0x00406c6d, and publishes the result in 0x00629f0c /
// 0x00629f04: the callee's edi/eax when it reports CF (a segment was found), else the remembered
// start. ebp, ebx, ecx, edx and esi are saved and restored; the flags are the callee's.
void __cdecl FUN_00406d4b(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x006278e8) = r->edi;
    GP4_GLOBAL(uint32_t, 0x006278ec) = r->eax;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x00627948);
    // cmp byte [0x627948], 0
    call.eflags = (r->eflags & ~0x8d5u) | parityFlag(mode) | (mode == 0u ? 0x40u : 0u) | (mode & 0x80u);
    if (mode != 0u) {
        searchForward(call);
    } else {
        searchBackward(call);
    }

    uint32_t edi;
    uint32_t eax;
    if ((call.eflags & 1u) != 0u) {
        edi = call.edi;
        eax = call.eax;
    } else {
        edi = GP4_GLOBAL(uint32_t, 0x006278e8);
        eax = GP4_GLOBAL(uint32_t, 0x006278ec);
    }
    GP4_GLOBAL(uint32_t, 0x00629f0c) = edi;
    GP4_GLOBAL(uint32_t, 0x00629f04) = eax;

    r->eax = eax;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00406d4b, FUN_00406d4b, "regs(eax:u32, edi:rec[0x8943a0,0x160,0..14]) -> (eax, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_00406ac7+ring_0040aec5+state_004067ea+scan_00406d4b")
