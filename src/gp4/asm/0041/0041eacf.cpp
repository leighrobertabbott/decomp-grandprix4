#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// 0x0041eaa7: eax = -round(record.dword[0x150] * DAT_006f2a04 * esi) for the record in edi.
static inline uint32_t timeOffset(gp4::Regs& c, uint32_t record) {
    c.st_in = 0;
    c.st_out = 0;
    c.edi = record;
    gp4::call_regs(0x0041eaa7, c);
    return c.eax;
}

// 0x0041ea89: eax = max(0x40000 - ((esi * record.word[0x14]) >> 14), 0x10); edx = high half of the product.
static inline uint32_t timeScale(gp4::Regs& c, uint32_t record) {
    c.st_in = 0;
    c.st_out = 0;
    c.edi = record;
    gp4::call_regs(0x0041ea89, c);
    return c.eax;
}

static inline uint32_t recordDword(uint32_t record) {
    return GP4_FIELD(uint32_t, reinterpret_cast<void*>(static_cast<uintptr_t>(record)), 0xf4);
}

// edi = one of the 0x160-byte track-segment records at 0x008943a0 (the lap ring below the pit start
// [0x891500], the pit ring from it), edx = the factor handed on to 0x0041eaa7 / 0x0041ea89 as esi.
// Records near the lap start or the lap end (the first one, or at/above [0x866ecc] - 0x2c0) are handled by
// 0x0041e910. Otherwise the four records record-1, record, record+1 and the one after it (wrapping through
// the pit ring end [0x866ef8] -> [0x866f04] or the lap end [0x866ecc] -> 0x8943a0) give
//   0x00637ff4 = offset(record)   + record.dword[0xf4]    0x0062785c = scale(record)
//   0x00637ff0 = offset(record-1) + (record-1).dword[0xf4]  0x00627858 = scale(record-1)
//   0x00637ff8 = offset(record+1) + (record+1).dword[0xf4]  0x00627860 = scale(record+1)
//   0x00637ffc = offset(next)     + next.dword[0xf4]
// edi and esi are restored; eax is the last stored sum, edx the high product word of the last scale call
// and the flags those of the last add (or of 0x0041e910).
// FUNCTION: GP4 0x0041eacf
void __cdecl FUN_0041eacf(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.esi = r->edx;
    uint32_t record = r->edi;

    if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
        if (record == 0x008943a0u || record >= GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x2c0u) {
            c.st_in = 0;
            c.st_out = 0;
            gp4::call_regs(0x0041e910, c);
            r->eax = c.eax;
            r->edx = c.edx;
            r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
            return;
        }
    }

    uint32_t offset = timeOffset(c, record);
    uint32_t sum = offset + recordDword(record);
    GP4_GLOBAL(uint32_t, 0x00637ff4) = sum;
    GP4_GLOBAL(uint32_t, 0x0062785c) = timeScale(c, record);

    record -= 0x160u;
    offset = timeOffset(c, record);
    sum = offset + recordDword(record);
    GP4_GLOBAL(uint32_t, 0x00637ff0) = sum;
    GP4_GLOBAL(uint32_t, 0x00627858) = timeScale(c, record);

    record += 0x2c0u;
    offset = timeOffset(c, record);
    sum = offset + recordDword(record);
    GP4_GLOBAL(uint32_t, 0x00637ff8) = sum;
    GP4_GLOBAL(uint32_t, 0x00627860) = timeScale(c, record);

    record -= 0x160u;
    if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
        record += 0x160u;
        if (record >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            record = 0x008943a0u;
        }
    } else {
        record += 0x160u;
        if (record >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            record = GP4_GLOBAL(uint32_t, 0x00866f04);
        }
    }
    record += 0x160u;

    offset = timeOffset(c, record);
    const uint32_t field = recordDword(record);
    sum = offset + field;
    GP4_GLOBAL(uint32_t, 0x00637ffc) = sum;

    r->eax = sum;
    r->edx = c.edx;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(offset, field);
}
GP4_IMPL(0x0041eacf, FUN_0041eacf, "regs(edi:rec[0x8943a0,0x160,0..25], edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+multiply_0040ce7b+dvar_0041eacf")
