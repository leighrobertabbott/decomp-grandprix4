#include <gp4/gp4.h>

// Flags of "sub left, right" (32-bit).
static inline uint32_t subtractFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUN_00405e19 on the record at `record`: returns its CF (sign of the dot product) and leaves
// its flags in call.eflags.
static inline bool probeRecord(gp4::Regs& call, uint32_t record) {
    call.edi = record;
    gp4::call_regs(0x00405e19, call);
    return (call.eflags & 1u) != 0u;
}

static inline bool recordHasBit3(uint32_t record) {
    return (GP4_FIELD(uint8_t, reinterpret_cast<void*>(static_cast<uintptr_t>(record)), 0x92) & 8u) != 0u;
}

// FUNCTION: GP4 0x00405da9
void __cdecl FUN_00405da9(gp4::Regs* r) {
    // push/pop of ebx, ecx, edx, ebp, esi and edi restore them; eax returns the record found and
    // the flags are those of the last flag writer on the path taken.
    gp4::Regs call = *r;
    call.ecx = static_cast<uint32_t>(static_cast<int32_t>(r->eax) >> 11);
    call.ebx = static_cast<uint32_t>(static_cast<int32_t>(r->edx) >> 11);
    uint32_t record = r->edi;
    uint32_t flags;

    if (probeRecord(call, record)) {
        // walk backwards while the probe keeps its carry
        bool carry;
        do {
            if (recordHasBit3(record)) {
                call.edi = record;
                gp4::call_regs(0x00406077, call);
                record = call.edi;
            } else {
                record -= 0x160u;
            }
            carry = probeRecord(call, record);
        } while (carry);
        flags = call.eflags;
    } else {
        // walk forwards until the probe sets its carry
        bool carry;
        do {
            record += 0x160u;
            if (recordHasBit3(record)) {
                call.edi = record;
                gp4::call_regs(0x00406051, call);
                record = call.edi;
            }
            carry = probeRecord(call, record);
        } while (!carry);
        if (recordHasBit3(record)) {
            call.edi = record;
            gp4::call_regs(0x00406077, call);
            record = call.edi;
            flags = call.eflags;
        } else {
            flags = subtractFlags32(record, 0x160u);
            record -= 0x160u;
        }
    }

    r->eax = record;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00405da9, FUN_00405da9, "regs(eax:i32[-0x4000000..0x3fff7ff], edx:i32[-0x4000000..0x3fff7ff], edi:rec[0x8943a0,0x160,0..11]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12")
