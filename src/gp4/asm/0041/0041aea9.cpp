#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `test`/`or`/`and` with a result of Bits width: CF = OF = AF = 0.
template <unsigned Bits>
static inline uint32_t logicFlags(uint32_t result) {
    const uint32_t mask = 0xffffffffu >> (32u - Bits);
    result &= mask;
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & (1u << (Bits - 1u))) != 0u ? 0x80u : 0u);
}

// Flags of `cmp left, right` / `sub` with operands of Bits width.
template <unsigned Bits>
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t mask = 0xffffffffu >> (32u - Bits);
    const uint32_t sign = 1u << (Bits - 1u);
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & sign) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & sign) != 0u ? 0x800u : 0u);
}

static inline void finish(gp4::Regs* r, uint32_t eax, uint32_t ebx, uint32_t edx, uint32_t flags) {
    r->eax = eax;
    r->ebx = ebx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// Runs one of the two helper routines (0x0041afb1 / 0x0041af84) with the current eax/ebx/edx; they
// leave eax and edx changed and return CF in the flags.
static inline uint32_t callHelper(gp4::Regs* r, uint32_t addr, uint32_t& eax, uint32_t ebx, uint32_t& edx) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = eax;
    call.ebx = ebx;
    call.edx = edx;
    gp4::call_regs(addr, call);
    eax = call.eax;
    edx = call.edx;
    return call.eflags & 0x8d5u;
}

// FUNCTION: GP4 0x0041aea9
// Decides whether the car record in esi should raise bit 0x10 of the byte at +0x6f (and record the cause
// in DAT_00638dc4: 0x40 for the speed test, 0x80 for the table test). Does nothing when bit 1 of +0xe5 is
// set. With +0x72 == 0 the bit is raised when +0x6f bit 0 is set and +0x7b bit 0x20 is clear. Otherwise,
// with the word +0x62 at or above DAT_0060131c: a helper (0x41afb1) may veto when +0xe5 bit 7 is set,
// then |word +0x9a| >= word DAT_0060383c and +0x72 != +0x1cd raise it (cause 0x40). Below the limit:
// word +0x62 against the table at 0x7c28bc (row (+0x7c & 0x3f) - 1 clamped to 0x15, column |+0x72|),
// then +0x6f bit 1 / +0xe5 bit 7, +0x72 != 1 and helper 0x41af84 (CF clear) raise it (cause 0x80).
// eax, ebx, edx and the flags are exactly what the original leaves at each of its exits.
void __cdecl FUN_0041aea9(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t edx = r->edx;

    const uint8_t status = GP4_FIELD(uint8_t, body, 0xe5);
    if ((status & 2u) != 0u) {
        finish(r, eax, ebx, edx, logicFlags<8>(status & 2u));
        return;
    }

    const uint8_t level = GP4_FIELD(uint8_t, body, 0x72);
    if (level == 0u) {
        const uint8_t flagsByte = GP4_FIELD(uint8_t, body, 0x6f);
        if ((flagsByte & 1u) == 0u) {
            finish(r, eax, ebx, edx, logicFlags<8>(flagsByte & 1u));
            return;
        }
        const uint8_t other = GP4_FIELD(uint8_t, body, 0x7b);
        if ((other & 0x20u) != 0u) {
            finish(r, eax, ebx, edx, logicFlags<8>(other & 0x20u));
            return;
        }
        const uint8_t raised = static_cast<uint8_t>(flagsByte | 0x10u);
        GP4_FIELD(uint8_t, body, 0x6f) = raised;
        finish(r, eax, ebx, edx, logicFlags<8>(raised));
        return;
    }

    const int32_t position = GP4_FIELD(int16_t, body, 0x62);
    eax = static_cast<uint32_t>(position);
    const int32_t limit = GP4_GLOBAL(int32_t, 0x0060131c);

    if (position >= limit) {
        uint32_t flags = subFlags<32>(eax, static_cast<uint32_t>(limit));
        if ((status & 0x80u) != 0u) {
            flags = callHelper(r, 0x0041afb1, eax, ebx, edx);
            if ((flags & 1u) != 0u) {
                finish(r, eax, ebx, edx, flags);
                return;
            }
        } else {
            flags = logicFlags<8>(status & 0x80u);
        }

        uint32_t speed = GP4_FIELD(uint16_t, body, 0x9a);
        if ((speed & 0x8000u) != 0u) {
            speed = (0u - speed) & 0xffffu;
        }
        eax = (eax & 0xffff0000u) | speed;
        const uint32_t threshold = GP4_GLOBAL(uint16_t, 0x0060383c);
        flags = subFlags<16>(speed, threshold);
        if (static_cast<int16_t>(speed) < static_cast<int16_t>(threshold)) {
            finish(r, eax, ebx, edx, flags);
            return;
        }

        const uint8_t reference = GP4_FIELD(uint8_t, body, 0x1cd);
        edx = (edx & 0xffffff00u) | reference;
        flags = subFlags<8>(level, reference);
        if (level == reference) {
            finish(r, eax, ebx, edx, flags);
            return;
        }
        const uint8_t raised = static_cast<uint8_t>(GP4_FIELD(uint8_t, body, 0x6f) | 0x10u);
        GP4_FIELD(uint8_t, body, 0x6f) = raised;
        GP4_GLOBAL(uint8_t, 0x00638dc4) = 0x40;
        finish(r, eax, ebx, edx, logicFlags<8>(raised));
        return;
    }

    int32_t row = static_cast<int32_t>(GP4_FIELD(uint8_t, body, 0x7c) & 0x3fu) - 1;
    if (row > 0x15) {
        row = 0x15;
    }
    ebx = static_cast<uint32_t>(row) << 4;

    uint32_t column = level;
    if ((column & 0x80u) != 0u) {
        column = (0u - column) & 0xffu;
    }
    edx = column;

    const uint32_t tableValue = *reinterpret_cast<const uint16_t*>(GP4_ARRAY(const uint8_t, 0x007c28bc) + (ebx + column * 2u));
    const uint32_t word = static_cast<uint16_t>(position);
    uint32_t flags = subFlags<16>(word, tableValue);
    if (static_cast<int16_t>(word) >= static_cast<int16_t>(tableValue)) {
        finish(r, eax, ebx, edx, flags);
        return;
    }

    if ((GP4_FIELD(uint8_t, body, 0x6f) & 2u) == 0u) {
        flags = logicFlags<8>(status & 0x80u);
        if ((status & 0x80u) != 0u) {
            finish(r, eax, ebx, edx, flags);
            return;
        }
    }

    flags = subFlags<8>(level, 1u);
    if (level == 1u) {
        finish(r, eax, ebx, edx, flags);
        return;
    }
    flags = callHelper(r, 0x0041af84, eax, ebx, edx);
    if ((flags & 1u) != 0u) {
        finish(r, eax, ebx, edx, flags);
        return;
    }
    const uint8_t raised = static_cast<uint8_t>(GP4_FIELD(uint8_t, body, 0x6f) | 0x10u);
    GP4_FIELD(uint8_t, body, 0x6f) = raised;
    GP4_GLOBAL(uint8_t, 0x00638dc4) = 0x80;
    finish(r, eax, ebx, edx, logicFlags<8>(raised));
}
GP4_IMPL(0x0041aea9, FUN_0041aea9, "regs(eax:u32, ebx:u32, edx:u32, esi:ptr[0x250]:layout=car_0041aea9) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041242f+thr_007c28bc")
