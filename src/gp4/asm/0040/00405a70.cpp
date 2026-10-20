#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "sub left, right" on a dword
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// bit 3 of the record byte at +0x92: the record is a ring boundary (0x4060b0 / 0x4060e6 hop over it)
static inline bool isRingEdge(uint32_t record) {
    return (GP4_FIELD(uint8_t, record, 0x92) & 8u) != 0u;
}

// FUNCTION: GP4 0x00405a70
// Walks the 0x160-byte records from edi until the projection 0x00405ad0 of the point
// (ecx >> 11, ebx >> 11) changes sign (CF = sign): forward while it is non-negative, ending one
// record back; backward while it is negative. Records with bit 3 of +0x92 use the ring-wrap
// helpers 0x004060b0 (forward) and 0x004060e6 (backward). ecx is left shifted, ebx is restored.
void __cdecl FUN_00405a70(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ecx = static_cast<uint32_t>(static_cast<int32_t>(r->ecx) >> 11);
    c.ebx = static_cast<uint32_t>(static_cast<int32_t>(r->ebx) >> 11);

    gp4::call_regs(0x00405ad0, c);
    if ((c.eflags & gp4::CF) == 0u) {
        do {
            c.edi += 0x160u;
            if (isRingEdge(c.edi)) {
                gp4::call_regs(0x004060b0, c);
            }
            gp4::call_regs(0x00405ad0, c);
        } while ((c.eflags & gp4::CF) == 0u);

        if (isRingEdge(c.edi)) {
            gp4::call_regs(0x004060e6, c);
        } else {
            c.eflags = (c.eflags & ~0x8d5u) | subFlags32(c.edi, 0x160u);
            c.edi -= 0x160u;
        }
    } else {
        do {
            if (isRingEdge(c.edi)) {
                gp4::call_regs(0x004060e6, c);
            } else {
                c.edi -= 0x160u;
            }
            gp4::call_regs(0x00405ad0, c);
        } while ((c.eflags & gp4::CF) != 0u);
    }

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00405a70, FUN_00405a70, "regs(ecx:i32[-0x4000000..0x3fff7ff], ebx:i32[-0x4000000..0x3fff7ff], edi:rec[0x8943a0,0x160,0..11]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12")
