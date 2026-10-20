#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `dec reg` on the 32-bit value `before`: every arithmetic flag but CF, which keeps the
// value `carry` left by the previous instruction.
static inline uint32_t decFlags32(uint32_t before, uint32_t carry) {
    const uint32_t result = before - 1u;
    return (carry & 1u) | parityFlag(result) | (((before ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (before == 0x80000000u ? 0x800u : 0u);
}

static inline uint8_t recordFlagsByte(uint32_t record) {
    return *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(record) + 0x92u);
}

// 0x00405e19 for the record at `record` and the point (DAT_00623e30, DAT_00623e38): true when the
// signed dot product is negative (CF); `flags` receives the call's arithmetic flags.
static bool sideIsNegative(const gp4::Regs* r, uint32_t record, uint32_t* flags) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edi = record;
    call.ecx = GP4_GLOBAL(uint32_t, 0x00623e30);
    call.ebx = GP4_GLOBAL(uint32_t, 0x00623e38);
    gp4::call_regs(0x00405e19, call);
    *flags = call.eflags & 0x8d5u;
    return (call.eflags & 1u) != 0u;
}

// 0x00406051: remaps a record pointer that has just advanced past a flagged record.
static uint32_t remapForward(const gp4::Regs* r, uint32_t record) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edi = record;
    gp4::call_regs(0x00406051, call);
    return call.edi;
}

// 0x00406077: steps a record pointer back one record through the ring wrap; `carry` is its CF.
static uint32_t stepBackRing(const gp4::Regs* r, uint32_t record, uint32_t* carry) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edi = record;
    gp4::call_regs(0x00406077, call);
    *carry = call.eflags & 1u;
    return call.edi;
}

// Walks the ring of 0x160-byte records from the record in edi until the sign of the dot product
// computed by 0x00405e19 for the point (DAT_00623e30, DAT_00623e38) flips. eax = signed number of
// steps taken (the last step is taken back), edx = the record reached; edi, ebx, ecx are preserved.
// The flags are those of the final `dec` (positive start) or of the last 0x00405e19 (negative start).
// FUNCTION: GP4 0x004061ed
void __cdecl FUN_004061ed(gp4::Regs* r) {
    uint32_t record = r->edi;
    uint32_t count = 0;
    uint32_t flags;

    if (!sideIsNegative(r, record, &flags)) {
        do {
            record += 0x160u;
            if ((recordFlagsByte(record) & 8u) != 0u) {
                record = remapForward(r, record);
            }
            count++;
        } while (!sideIsNegative(r, record, &flags));

        uint32_t carry;
        if ((recordFlagsByte(record) & 8u) != 0u) {
            record = stepBackRing(r, record, &carry);
        } else {
            carry = record < 0x160u ? 1u : 0u;
            record -= 0x160u;
        }
        flags = decFlags32(count, carry);
        count--;
    } else {
        do {
            if ((recordFlagsByte(record) & 8u) != 0u) {
                uint32_t carry;
                record = stepBackRing(r, record, &carry);
            } else {
                record -= 0x160u;
            }
            count--;
        } while (sideIsNegative(r, record, &flags));
    }

    r->eax = count;
    r->edx = record;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004061ed, FUN_004061ed, "regs(edi:rec[0x8943a0,0x160,0..11]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12")
