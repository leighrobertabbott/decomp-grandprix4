#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00406ebc
// Settles a position (eax, edx = x, z; edi = a track segment, a pit record being replaced by its link
// [edi + 0x44]) on the segment ring: [0x6278c0] / [0x6278c8] = position, 0x00406ac7 finds the record
// (result in 0x00629f04 / 0x00629f0c) and 0x004067b7 (slot ebx = 0) refines it. Leaves edi = [0x629f0c]
// and eax = [0x629f04]; when |eax| >= 2 the pair is passed through 0x00406d4b. edx = |eax|; ebx is
// preserved.
void __cdecl FUN_00406ebc(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x006278c0) = r->eax;
    GP4_GLOBAL(uint32_t, 0x006278c8) = r->edx;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    if (s.edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        s.edi = GP4_FIELD(uint32_t, s.edi, 0x44);
    }
    gp4::call_regs(0x00406ac7, s);
    s.ebx = 0;
    gp4::call_regs(0x004067b7, s);

    s.edi = GP4_GLOBAL(uint32_t, 0x00629f0c);
    s.eax = GP4_GLOBAL(uint32_t, 0x00629f04);
    s.edx = s.eax;
    if (static_cast<int32_t>(s.edx) < 0) {
        s.edx = 0u - s.edx;
    }
    s.eflags = (s.eflags & ~0x8d5u) | cmpFlags32(s.edx, 2u);
    if (static_cast<int32_t>(s.edx) >= 2) {
        gp4::call_regs(0x00406d4b, s);
    }

    r->eax = s.eax;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->edi = s.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x00406ebc, FUN_00406ebc, "regs(eax:u32, edx:u32, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_004381ab")
