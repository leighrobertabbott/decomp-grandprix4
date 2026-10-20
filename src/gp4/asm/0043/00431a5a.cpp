#include <gp4/gp4.h>

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00431a5a_Esi {
    uint8_t Unknown00[0x70];
    uint8_t Unknown70;
    uint8_t Unknown71[0x136 - 0x71];
    uint8_t Unknown136;
    uint8_t Unknown137[0x198 - 0x137];
    uint8_t Unknown198;
    uint8_t Unknown199;
    uint8_t Unknown19A;
    uint8_t Unknown19B;
};

// FUNCTION: GP4 0x00431a5a
// al (+1) is a divisor n kept in byte 0x198. With a = word DAT_007c2e96 and d = byte esi[0x70]:
// span = ((max(a - d, 0) + 1) << 6), averaged over n as (2*span + n) / n >> 1 (span when n == 1).
// Bytes 0x199/0x19a/0x19b receive (d*64 + span - 0x40, clamped at 0, then + span each) >> 6.
// eax = min(span, 0x18c0) clamped at 0, plus 0x80; the flags are those of that last add.
void __cdecl FUN_00431a5a(gp4::Regs* r) {
    Unknown_00431a5a_Esi* record = reinterpret_cast<Unknown_00431a5a_Esi*>(static_cast<uintptr_t>(r->esi));
    const uint32_t input = r->eax;

    record->Unknown198 = static_cast<uint8_t>(input);
    record->Unknown136 = 0;
    const uint32_t divisor = (input & 0xffu) + 1u;

    int32_t difference = static_cast<int32_t>(GP4_GLOBAL(uint16_t, 0x007c2e96)) - static_cast<int32_t>(record->Unknown70);
    if (difference < 0) {
        difference = 0;
    }
    uint32_t span = static_cast<uint32_t>(difference + 1) << 6;
    if (divisor != 1u) {
        span = ((span << 1) + divisor) / divisor;
        span >>= 1;
    }

    int32_t start = static_cast<int32_t>((static_cast<uint32_t>(record->Unknown70) << 6) + span - 0x40u);
    if (start < 0) {
        start = 0;
    }
    uint32_t cursor = static_cast<uint32_t>(start);
    record->Unknown199 = static_cast<uint8_t>(cursor >> 6);
    cursor += span;
    record->Unknown19A = static_cast<uint8_t>(cursor >> 6);
    cursor += span;
    record->Unknown19B = static_cast<uint8_t>(cursor >> 6);

    int32_t clamped = static_cast<int32_t>(span);
    if (clamped > 0x18c0) {
        clamped = 0x18c0;
    }
    if (clamped < 0) {
        clamped = 0;
    }
    const uint32_t value = static_cast<uint32_t>(clamped);

    r->eax = value + 0x80u;
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(value, 0x80u);
}
GP4_IMPL(0x00431a5a, FUN_00431a5a, "regs(eax:u32, esi:ptr[0x1a8]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
