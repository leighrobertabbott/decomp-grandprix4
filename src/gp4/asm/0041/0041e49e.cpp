#include <gp4/gp4.h>

struct Unknown_0041e49e {
    uint8_t Unknown00[0x50];
    uint32_t Unknown50;
    uint8_t Unknown54[0x71 - 0x54];
    uint8_t Flags71;
    uint8_t Unknown72[0x81 - 0x72];
    uint8_t Flags81;
    uint8_t Unknown82[0x9a - 0x82];
    int16_t Unknown9A;
    uint8_t Unknown9C[0x21c - 0x9c];
    uint8_t Flags21C;
    uint8_t Unknown21D[0x232 - 0x21d];
    uint8_t Flags232;
    uint8_t Unknown233[0x273 - 0x233];
    uint8_t Flags273;
    uint8_t Unknown274[0x2a0 - 0x274];
    uint32_t Unknown2A0;
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

// FUNCTION: GP4 0x0041e49e
// If bit 2 of +0x71 is clear and the signed word at +0x9a is at least [0x6039f0]: flag the record
// (+0x81 |= 0x10, +0x21c |= 0x20), store table[word 0x7ad89e] + [0x66d844] at +0x50 and the
// deadline [0x66d814] + [0x602acc] at +0x2a0 (eax = [0x602acc], flags of that add). Otherwise
// +0x273 |= 2 and +0x232 &= 0xfd (flags of the AND); eax is the sign-extended word when the first
// test passed (the movsx runs before the compare).
void __cdecl FUN_0041e49e(gp4::Regs* r) {
    Unknown_0041e49e* rec = reinterpret_cast<Unknown_0041e49e*>(static_cast<uintptr_t>(r->esi));

    if ((rec->Flags71 & 4u) == 0u) {
        const int32_t word = rec->Unknown9A;
        r->eax = static_cast<uint32_t>(word);
        if (word >= GP4_GLOBAL(int32_t, 0x006039f0)) {
            rec->Flags81 = static_cast<uint8_t>(rec->Flags81 | 0x10u);
            rec->Flags21C = static_cast<uint8_t>(rec->Flags21C | 0x20u);

            const uint32_t index = GP4_GLOBAL(uint16_t, 0x007ad89e);
            rec->Unknown50 = GP4_ARRAY(uint32_t, 0x00602ab8)[index] + GP4_GLOBAL(uint32_t, 0x0066d844);

            const uint32_t delay = GP4_GLOBAL(uint32_t, 0x00602acc);
            const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
            rec->Unknown2A0 = now + delay;

            r->eax = delay;
            r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(now, delay);
            return;
        }
    }

    rec->Flags273 = static_cast<uint8_t>(rec->Flags273 | 2u);
    const uint32_t cleared = rec->Flags232 & 0xfdu;
    rec->Flags232 = static_cast<uint8_t>(cleared);

    // AND: CF = OF = AF = 0, PF/ZF/SF from the byte
    const uint32_t flags = parityFlag8(cleared)
        | (cleared == 0u ? 0x40u : 0u)
        | (cleared & 0x80u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041e49e, FUN_0041e49e, "regs(esi:ptr[0x2ac]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
