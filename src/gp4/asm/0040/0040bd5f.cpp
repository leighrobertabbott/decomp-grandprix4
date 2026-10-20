#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// cmp left, right (32-bit) arithmetic flags.
static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// cmp left, right (8-bit) arithmetic flags.
static inline uint32_t subtractionFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// test of a 32-bit result: CF = OF = AF = 0.
static inline uint32_t testFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// test of a byte result: CF = OF = AF = 0.
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// |value - subtrahend| as a 32-bit sub / jns / neg.
static inline uint32_t absoluteDifference(uint32_t value, uint32_t subtrahend) {
    uint32_t difference = value - subtrahend;
    if (static_cast<int32_t>(difference) < 0) {
        difference = 0u - difference;
    }
    return difference;
}

static inline void finish(gp4::Regs* r, uint32_t eax, uint32_t ebx, uint32_t edx, uint32_t ebp, uint32_t flags) {
    r->eax = eax;
    r->ebx = ebx;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// FUNCTION: GP4 0x0040bd5f
// Per-slot update of the 10-byte record at 0x00675a82 + 10 * word[edi + 0x7a] (edi and ecx are kept),
// skipped when bit 1 of [edi + 0x70] is set. Sets the byte DAT_0062ac98 to 0xff when it changed the
// record. Tables indexed by the dword [0x0062258c]: step 0x006225c0, limit 0x006225a8. The record's
// byte +4 is lowered by 4 * step (floored at 0); byte +0 moves toward +2 by the step and byte +1
// toward the limit (down through the byte pair table 0x006225d8, masked by [0x0063b81c], when above
// it), both only while the neighbouring records (-0xa/-9 and +0xa/+0xb) stay within 0x10 of the new
// values; byte +3 follows byte +0 by [0x0062257c]. Exits leave eax/ebx/edx/ebp and the flags of the
// last compare, which differ per exit.
void __cdecl FUN_0040bd5f(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;

    GP4_GLOBAL(uint8_t, 0x0062ac98) = 0;
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x0062258c);
    const uint32_t* const steps = GP4_ARRAY(uint32_t, 0x006225c0);
    const uint32_t* const limits = GP4_ARRAY(uint32_t, 0x006225a8);

    const uint8_t gate = GP4_FIELD(uint8_t, source, 0x70);
    if ((gate & 2u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | testFlags8(gate & 2u);
        return;
    }

    uint8_t* record = GP4_ARRAY(uint8_t, 0x00675a82) + static_cast<uint32_t>(GP4_FIELD(const uint16_t, source, 0x7a)) * 10u;
    record[5] = 0;

    eax = record[4];
    edx = steps[index] << 2;
    eax -= edx;
    if (static_cast<int32_t>(eax) < 0) {
        eax = 0;
    }
    if (record[4] != static_cast<uint8_t>(eax)) {
        record[4] = static_cast<uint8_t>(eax);
        GP4_GLOBAL(uint8_t, 0x0062ac98) = 0xff;
    }

    edx = steps[index];
    ebp = record[2];
    ebx = record[0];
    if (ebx < ebp) {
        eax = ebx + edx;
        if (static_cast<int32_t>(eax) > static_cast<int32_t>(ebp)) {
            eax = ebp;
        }
        edx = absoluteDifference(record[-0xa], eax);
        if (static_cast<int32_t>(edx) > 0x10) {
            finish(r, eax, ebx, edx, ebp, subtractionFlags32(edx, 0x10u));
            return;
        }
        edx = absoluteDifference(record[0xa], eax);
        if (static_cast<int32_t>(edx) > 0x10) {
            finish(r, eax, ebx, edx, ebp, subtractionFlags32(edx, 0x10u));
            return;
        }
        ebx = eax;
    }

    eax = record[1];
    if (static_cast<int32_t>(eax) > static_cast<int32_t>(limits[index])) {
        edx = eax >> 4;
        if (static_cast<int32_t>(edx) > 0xb) {
            edx = 0xb;
        }
        ebp = GP4_ARRAY(const uint8_t, 0x006225d9)[edx * 2u];
        const uint32_t masked = GP4_GLOBAL(uint32_t, 0x0063b81c) & ebp;
        if (masked != 0u) {
            finish(r, eax, ebx, edx, ebp, testFlags32(masked));
            return;
        }
        ebp = GP4_ARRAY(const uint8_t, 0x006225d8)[edx * 2u];
        eax -= ebp;
        if (static_cast<int32_t>(eax) < static_cast<int32_t>(limits[index])) {
            eax = limits[index];
        }
    } else {
        eax += steps[index];
        if (static_cast<int32_t>(eax) > static_cast<int32_t>(limits[index])) {
            eax = limits[index];
        }
    }

    edx = absoluteDifference(record[-9], eax);
    if (static_cast<int32_t>(edx) > 0x10) {
        finish(r, eax, ebx, edx, ebp, subtractionFlags32(edx, 0x10u));
        return;
    }
    edx = absoluteDifference(record[0xb], eax);
    if (static_cast<int32_t>(edx) > 0x10) {
        finish(r, eax, ebx, edx, ebp, subtractionFlags32(edx, 0x10u));
        return;
    }

    if (record[0] != static_cast<uint8_t>(ebx)) {
        record[0] = static_cast<uint8_t>(ebx);
        GP4_GLOBAL(uint8_t, 0x0062ac98) = 0xff;
    }
    if (record[1] != static_cast<uint8_t>(eax)) {
        record[1] = static_cast<uint8_t>(eax);
        GP4_GLOBAL(uint8_t, 0x0062ac98) = 0xff;
    }
    GP4_GLOBAL(uint8_t, 0x00622529) = static_cast<uint8_t>(eax);

    edx = record[0];
    eax = record[3] + GP4_GLOBAL(uint32_t, 0x0062257c);
    if (static_cast<int32_t>(eax) > static_cast<int32_t>(edx)) {
        eax = edx;
    }
    const uint32_t flags = subtractionFlags8(record[3], static_cast<uint8_t>(eax));
    if (record[3] != static_cast<uint8_t>(eax)) {
        record[3] = static_cast<uint8_t>(eax);
        GP4_GLOBAL(uint8_t, 0x0062ac98) = 0xff;
    }
    finish(r, eax, ebx, edx, ebp, flags);
}
GP4_IMPL(0x0040bd5f, FUN_0040bd5f, "regs(edi:ptr[0x84]:layout=rec_0040bd5f, eax:u32, ebx:u32, edx:u32, ebp:u32) -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0040bec7+state_0040bd5f")
