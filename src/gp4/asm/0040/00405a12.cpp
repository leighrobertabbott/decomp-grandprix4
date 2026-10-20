#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subtractFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline bool carrySet(const gp4::Regs& s) { return (s.eflags & 1u) != 0u; }

// Bit 3 of the record flag byte +0x92 marks a record that sits at a ring boundary.
static inline bool isRingEdge(uint32_t record) {
    return (GP4_FIELD(uint8_t, reinterpret_cast<void*>(static_cast<uintptr_t>(record)), 0x92) & 8u) != 0u;
}

// edi = a 0x160-byte record of the ring at 0x008943a0, ecx / ebx = a 2D point (>> 11 here).
// 0x405ad0 classifies the point against a record (CF); walks the ring forward (CF clear) or
// backward (CF set) one record at a time, wrapping through 0x406051 / 0x406077 at ring edges.
// FUNCTION: GP4 0x00405a12
void __cdecl FUN_00405a12(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.ecx = static_cast<uint32_t>(static_cast<int32_t>(r->ecx) >> 11);
    s.ebx = static_cast<uint32_t>(static_cast<int32_t>(r->ebx) >> 11);
    gp4::call_regs(0x00405ad0, s);

    if (!carrySet(s)) {
        do {
            s.edi += 0x160u;
            if (isRingEdge(s.edi)) gp4::call_regs(0x00406051, s);
            gp4::call_regs(0x00405ad0, s);
        } while (!carrySet(s));

        if (isRingEdge(s.edi)) {
            gp4::call_regs(0x00406077, s);
        } else {
            const uint32_t before = s.edi;
            s.edi = before - 0x160u;
            s.eflags = (s.eflags & ~0x8d5u) | subtractFlags32(before, 0x160u);
        }
    } else {
        do {
            if (isRingEdge(s.edi)) {
                gp4::call_regs(0x00406077, s);
            } else {
                const uint32_t before = s.edi;
                s.edi = before - 0x160u;
                s.eflags = (s.eflags & ~0x8d5u) | subtractFlags32(before, 0x160u);
            }
            gp4::call_regs(0x00405ad0, s);
        } while (carrySet(s));
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->edi = s.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x00405a12, FUN_00405a12, "regs(ebx:i32[-0x4000000..0x3fff7ff], ecx:i32[-0x4000000..0x3fff7ff], edi:rec[0x8943a0,0x160,0..11]) -> (eax, ebx, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12")
