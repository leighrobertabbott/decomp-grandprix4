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

// Flags of a 32-bit logic result (test / and): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Kept as separate call sites so the shape gate sees both helpers.
static __declspec(noinline) void leftEdge(gp4::Regs& s) { gp4::call_regs(0x0040cf40, s); }
static __declspec(noinline) void rightEdge(gp4::Regs& s) { gp4::call_regs(0x0040cf52, s); }

// esi = a track-segment record. For the record itself and the next one (+0x160) the two edge points
// (0x0040cf40 with the left length, 0x0040cf52 with the right length; edi = the record) are stored as
// (eax, edx) pairs at 0x00627908/18, 0x0062790c/1c, 0x00627910/20 and 0x00627914/24. After each,
// 0x0040cfc9 settles the position against the caller's record edi and its |eax| >= 2 sets bit k
// (0..3) of the state dword 0x00627940. Carry is set (stc) when the four bits are not all set and
// either bit 0 of the state dword 0x0062793c is set or its bits 1 and 2 both are; the flags are the
// ones of the last compare / test, with CF forced.
// FUNCTION: GP4 0x0040d01b
void __cdecl FUN_0040d01b(gp4::Regs* r) {
    const uint32_t record = r->esi;
    const uint32_t callerEdi = r->edi;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    for (uint32_t step = 0; step < 4u; ++step) {
        s.edi = record + ((step & 2u) != 0u ? 0x160u : 0u);
        if ((step & 1u) == 0u) {
            leftEdge(s);
        } else {
            rightEdge(s);
        }
        s.edi = callerEdi;
        GP4_ARRAY(uint32_t, 0x00627908)[step] = s.eax;
        GP4_ARRAY(uint32_t, 0x00627918)[step] = s.edx;
        gp4::call_regs(0x0040cfc9, s);
        if (static_cast<int32_t>(s.eax) < 0) {
            s.eax = 0u - s.eax;
        }
        if (static_cast<int32_t>(s.eax) >= 2) {
            GP4_GLOBAL(uint32_t, 0x00627940) |= 1u << step;
        }
    }

    uint32_t flags;
    const uint32_t state = GP4_GLOBAL(uint32_t, 0x00627940);
    if (state == 0xfu) {
        flags = cmpFlags32(state, 0xfu) & ~1u;
    } else {
        const uint32_t word = GP4_GLOBAL(uint32_t, 0x0062793c);
        s.eax = word;
        if ((word & 1u) != 0u) {
            flags = logicFlags32(word & 1u) | 1u;
        } else {
            s.eax = word & 6u;
            flags = cmpFlags32(s.eax, 6u);
            if (s.eax == 6u) {
                flags |= 1u;
            } else {
                flags &= ~1u;
            }
        }
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0040d01b, FUN_0040d01b, "regs(esi:rec[0x8943a0,0x160,0..24], edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+ptr_00627938")
