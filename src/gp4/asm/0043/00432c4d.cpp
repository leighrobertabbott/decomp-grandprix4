#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `add word ptr [m], reg16` (CF PF AF ZF SF OF).
static inline uint32_t addFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left + right) & 0xffffu;
    return (((left + right) & 0x10000u) != 0u ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Moves the word at +delta toward the word at +target by at most the magnitude of the word at
// +limit (a signed 16-bit slew limiter). Returns the final edx / ebp and the last sub result (eax).
struct SlewResult {
    uint32_t eax;
    uint32_t edx;
    uint32_t ebp;
    uint32_t flags;
};

static inline SlewResult slew(uint8_t* record, uint32_t targetOff, uint32_t currentOff, uint32_t limitOff) {
    const int32_t target = GP4_FIELD(int16_t, record, targetOff);
    const int32_t current = GP4_FIELD(int16_t, record, currentOff);
    const int32_t diff = target - current;
    int32_t magnitude = diff;
    if (diff < 0) {
        magnitude = -magnitude;
    }
    int32_t limit = GP4_FIELD(int16_t, record, limitOff);
    if (limit < 0) {
        limit = -limit;
    }
    if (limit > magnitude) {
        limit = magnitude;
    }
    if (diff < 0) {
        limit = -limit;
    }
    const uint32_t old = static_cast<uint16_t>(GP4_FIELD(int16_t, record, currentOff));
    GP4_FIELD(uint16_t, record, currentOff) = static_cast<uint16_t>(old + static_cast<uint32_t>(limit));

    SlewResult out;
    out.eax = static_cast<uint32_t>(diff);
    out.edx = static_cast<uint32_t>(limit);
    out.ebp = static_cast<uint32_t>(magnitude);
    out.flags = addFlags16(old, static_cast<uint32_t>(limit));
    return out;
}

// FUNCTION: GP4 0x00432c4d
void __cdecl FUN_00432c4d(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    slew(record, 0x280, 0x27e, 0x282);
    const SlewResult second = slew(record, 0x27a, 0x278, 0x27c);

    r->eax = second.eax;
    r->edx = second.edx;
    r->ebp = second.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | second.flags;
}
GP4_IMPL(0x00432c4d, FUN_00432c4d, "regs(esi:ptr[0x28a]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of)")
