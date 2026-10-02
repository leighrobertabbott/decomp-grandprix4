#include <gp4/gp4.h>

struct Unknown_0040ea46 {
    uint8_t Unknown00[0x415];
    uint8_t Unknown415;
};

struct Unknown_0066fef0 {
    uint8_t Unknown00[0x7c];
    uint8_t Unknown7C;
    uint8_t Unknown7D[0x41c - 0x7d];
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040ea46
void __cdecl FUN_0040ea46(gp4::Regs* r) {
    const Unknown_0040ea46* body = reinterpret_cast<const Unknown_0040ea46*>(r->esi);
    const uint32_t key = static_cast<uint32_t>(body->Unknown415) >> 1;

    uint32_t result = 0;
    // "cmp bl, 0" (key == 0) and an equal "cmp al, [record+0x7c]" both give ZF|PF.
    uint32_t flags = 0x44u;

    if (key != 0u) {
        const uint8_t wanted = GP4_ARRAY(uint8_t, 0x007c31db)[key];
        uint32_t record = 0x0066fef0u;
        flags = 0u;
        bool found = false;
        for (uint32_t remaining = 0x16u; remaining != 0u; remaining--) {
            const Unknown_0066fef0* entry = reinterpret_cast<const Unknown_0066fef0*>(static_cast<uintptr_t>(record));
            if (wanted == entry->Unknown7C) {
                found = true;
                flags = 0x44u;
                break;
            }
            flags = additionFlags32(record, 0x41cu);
            record += 0x41cu;
        }
        if (found) {
            result = record;
        }
    }

    r->ebx = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040ea46, FUN_0040ea46, "regs(esi:ptr[0x41d]:bytes) -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
