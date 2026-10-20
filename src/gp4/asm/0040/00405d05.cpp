#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) left by `sub left, right`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Bit 3 of the flag byte at +0x92 of the 0x160-byte record at `rec`
// (marks the records where the track list wraps to its other end).
static inline bool isWrapRecord(uint32_t rec) {
    void* p = reinterpret_cast<void*>(static_cast<uintptr_t>(rec));
    return (GP4_FIELD(uint8_t, p, 0x92) & 8u) != 0u;
}

// Steps back one record (0x4060e6 when the record is a wrap record, else sub edi, 0x160).
static inline void stepBack(gp4::Regs& c) {
    if (isWrapRecord(c.edi)) {
        gp4::call_regs(0x004060e6, c);
    } else {
        c.eflags = (c.eflags & ~0x8d5u) | subFlags(c.edi, 0x160u);
        c.edi -= 0x160u;
    }
}

// Walks the record list from the record at edi, using the sign of the dot product at 0x405e19 for
// the point (ecx, ebx) >> 11. Forward while the sign is clear, backward while it is set; ends with
// edi on the last record before the sign changes. ecx is left shifted (callers restore it).
// FUNCTION: GP4 0x00405d05
void __cdecl FUN_00405d05(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ecx = static_cast<uint32_t>(static_cast<int32_t>(r->ecx) >> 11);
    c.ebx = static_cast<uint32_t>(static_cast<int32_t>(r->ebx) >> 11);
    gp4::call_regs(0x00405e19, c);
    if ((c.eflags & 1u) != 0u) {
        do {
            stepBack(c);
            gp4::call_regs(0x00405e19, c);
        } while ((c.eflags & 1u) != 0u);
    } else {
        do {
            c.edi += 0x160u;
            if (isWrapRecord(c.edi)) {
                gp4::call_regs(0x004060b0, c);
            }
            gp4::call_regs(0x00405e19, c);
        } while ((c.eflags & 1u) == 0u);
        stepBack(c);
    }
    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00405d05, FUN_00405d05, "regs(ebx:i32[-0x4000000..0x3fff7ff], ecx:i32[-0x4000000..0x3fff7ff], edi:rec[0x8943a0,0x160,0..11]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12")
