#include <gp4/gp4.h>

struct Unknown_0040f175 {
    uint8_t Unknown00[0x60];
    int16_t Unknown60;
    uint8_t Unknown62[0xe];
    uint8_t Unknown70;
};

static inline uint32_t comparisonFlags(uint32_t left, uint32_t right, uint32_t sign, uint32_t mask) {
    const uint32_t result = (left - right) & mask;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & sign) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & sign) != 0u ? 0x800u : 0u);
}

static inline uint32_t callMarker(gp4::Regs* r) {
    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x0040ecab, callee);
    return callee.eflags & 0x8d5u;
}

// Runs 0x40ecab on the record at esi when its signed word at +0x60 is below 0x80 and the byte
// at +0x70 is below the word DAT_007c2e96 (or equal to it with the word at +0x60 below 0x40).
// eax and edx keep the zero-extended byte / word once they are loaded; the flags are those of
// the last compare, or of the callee.
// FUNCTION: GP4 0x0040f175
void __cdecl FUN_0040f175(gp4::Regs* r) {
    const Unknown_0040f175* rec = reinterpret_cast<const Unknown_0040f175*>(static_cast<uintptr_t>(r->esi));
    const uint32_t word = static_cast<uint32_t>(static_cast<uint16_t>(rec->Unknown60));
    uint32_t flags = comparisonFlags(word, 0x80u, 0x8000u, 0xffffu);

    if (rec->Unknown60 < 0x80) {
        const uint32_t level = rec->Unknown70;
        const uint32_t limit = GP4_GLOBAL(uint16_t, 0x007c2e96);
        r->eax = level;
        r->edx = limit;
        flags = comparisonFlags(level, limit, 0x80000000u, 0xffffffffu);
        bool run = false;
        if (static_cast<int32_t>(level) < static_cast<int32_t>(limit)) {
            run = true;
        } else if (static_cast<int32_t>(level) <= static_cast<int32_t>(limit)) {
            flags = comparisonFlags(word, 0x40u, 0x8000u, 0xffffu);
            if (rec->Unknown60 < 0x40) {
                run = true;
            }
        }
        if (run) {
            flags = callMarker(r);
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040f175, FUN_0040f175, "regs(esi:ptr[0x78]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
