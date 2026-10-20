#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of "sub left, right" (32-bit).
static inline uint32_t subtractFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Bit 3 of the flag byte at +0x92 of a 0x160-byte record marks a record at the end of the array
// that must be wrapped by 0x406051 (forward) or 0x406077 (backward).
static inline bool isWrapRecord(uint32_t record) {
    return (GP4_FIELD(uint8_t, reinterpret_cast<void*>(static_cast<uintptr_t>(record)), 0x92) & 8u) != 0u;
}

// Steps the record pointer in edi back one record (wrapping when the record is flagged); the
// flags are those of the last "sub edi, 0x160" (or the wrapper's).
static inline void stepBack(gp4::Regs& c) {
    if (isWrapRecord(c.edi)) {
        gp4::call_regs(0x00406077, c);
    } else {
        c.eflags = (c.eflags & ~0x8d5u) | subtractFlags32(c.edi, 0x160u);
        c.edi -= 0x160u;
    }
}

// Walks the 0x160-byte record array from the record at edi until the dot product computed by
// 0x405e19 for the point (ecx, ebx) >> 11 changes sign. A non-negative product walks forward
// (add edi, 0x160, wrapping flagged records through 0x406051) until the product is negative,
// then steps back once; a negative product walks backward until the product is non-negative.
// ecx and ebx are returned shifted; eax/edx are those of the last 0x405e19; the flags are those
// of the final instruction executed.
// FUNCTION: GP4 0x00405c9a
void __cdecl FUN_00405c9a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ecx = static_cast<uint32_t>(static_cast<int32_t>(c.ecx) >> 11);
    c.ebx = static_cast<uint32_t>(static_cast<int32_t>(c.ebx) >> 11);

    gp4::call_regs(0x00405e19, c);
    if ((c.eflags & 1u) == 0u) {
        do {
            c.edi += 0x160u;
            if (isWrapRecord(c.edi)) {
                gp4::call_regs(0x00406051, c);
            }
            gp4::call_regs(0x00405e19, c);
        } while ((c.eflags & 1u) == 0u);
        stepBack(c);
    } else {
        do {
            stepBack(c);
            gp4::call_regs(0x00405e19, c);
        } while ((c.eflags & 1u) != 0u);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00405c9a, FUN_00405c9a, "regs(ebx:i32[-0x4000000..0x3fff7ff], ecx:i32[-0x4000000..0x3fff7ff], edi:rec[0x8943a0,0x160,0..11]) -> (eax, ebx, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12")
