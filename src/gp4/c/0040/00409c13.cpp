#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

static inline uint32_t compareByteFlags(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

static inline uint32_t advanceFlags(uint32_t before, uint32_t after) {
    const uint32_t increment = 0x41cu;
    return (after < before ? 1u : 0u)
        | parityFlag(after)
        | (((before ^ increment ^ after) & 0x10u) != 0u ? 0x10u : 0u)
        | (after == 0u ? 0x40u : 0u)
        | ((after & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((~(before ^ increment)) & (before ^ after) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00409c13
void __cdecl FUN_00409c13(gp4::Regs* r) {
    const uint32_t incomingEax = r->eax;
    const uint8_t sought = GP4_GLOBAL(uint8_t, 0x0066d87e);
    GP4_GLOBAL(uint32_t, 0x0066d82c) = 0u;
    uint32_t flags = parityFlag(sought) | (sought == 0u ? 0x40u : 0u) | (sought & 0x80u);
    if (sought != 0u) {
        uint32_t remaining = 22u;
        uint8_t* record = GP4_ARRAY(uint8_t, 0x0066fef0);
        while (remaining != 0u) {
            const uint8_t identifier = record[0x7c];
            flags = compareByteFlags(sought, identifier);
            if (sought == identifier) break;
            const uint32_t before = reinterpret_cast<uint32_t>(record);
            record += 0x41c;
            flags = advanceFlags(before, reinterpret_cast<uint32_t>(record));
            --remaining;
        }
        r->ecx = remaining;
        r->esi = reinterpret_cast<uint32_t>(record);
        GP4_GLOBAL(uint32_t, 0x0066d82c) = r->esi;
    }
    r->eax = (incomingEax & 0xffffff00u) | sought;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00409c13, FUN_00409c13, "regs(eax:i32, ecx:i32, esi:i32) -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=layout=search_0066d82c")
