#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00431b00_Esi {
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

// (value * factor) >> 14 as "imul edx; shrd eax, edx, 14" leaves it in eax (low 32 bits of the shifted product).
static inline int32_t scaled(int32_t value, int32_t factor) {
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(factor);
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
}

// FUNCTION: GP4 0x00431b00
// pushal/popal wrapper: only memory and the flags change. With floor = max(eax - 0x80, 0) and
// span = (max(word DAT_007c2e96 - byte esi[0x70] + 1, 0)) << 6: floor >= span stores 0 in esi[0x198] (flags of
// that compare); otherwise esi[0x198] = n = 1 when floor >= 0.4166*span, 2 when floor >= 0.2917*span, else 3, and
// bytes 0x199/0x19a/0x19b receive (max(esi[0x70] - 1, 0)*64 + floor, then + (span - floor)/n each) >> 6. The
// flags of that path are those of the last "shr ebx, 6". Bytes 0x199..0x19b start at 0x6e, 0x136 is cleared.
void __cdecl FUN_00431b00(gp4::Regs* r) {
    Unknown_00431b00_Esi* record = reinterpret_cast<Unknown_00431b00_Esi*>(static_cast<uintptr_t>(r->esi));
    const int32_t input = static_cast<int32_t>(r->eax);

    record->Unknown136 = 0;
    record->Unknown199 = 0x6e;
    record->Unknown19A = 0x6e;
    record->Unknown19B = 0x6e;

    int32_t floor = input - 0x80;
    if (floor < 0) {
        floor = 0;
    }
    int32_t span = static_cast<int32_t>(GP4_GLOBAL(uint16_t, 0x007c2e96)) - static_cast<int32_t>(record->Unknown70) + 1;
    if (span < 0) {
        span = 0;
    }
    span = static_cast<int32_t>(static_cast<uint32_t>(span) << 6);

    uint32_t flags;
    if (!(floor < span)) {
        record->Unknown198 = 0;
        flags = subtractionFlags32(static_cast<uint32_t>(floor), static_cast<uint32_t>(span));
    } else {
        uint32_t divisor = 1;
        if (floor < scaled(span, 0x1aaa)) {
            divisor = 2;
            if (floor < scaled(span, 0x12aa)) {
                divisor = 3;
            }
        }
        record->Unknown198 = static_cast<uint8_t>(divisor);

        int32_t base = static_cast<int32_t>(record->Unknown70) - 1;
        if (base < 0) {
            base = 0;
        }
        uint32_t position = (static_cast<uint32_t>(base) << 6) + static_cast<uint32_t>(floor);
        record->Unknown199 = static_cast<uint8_t>(position >> 6);

        int32_t remaining = span - floor;
        if (remaining < 0) {
            remaining = 0;
        }
        const uint32_t step = static_cast<uint32_t>(remaining) / divisor;

        position += step;
        record->Unknown19A = static_cast<uint8_t>(position >> 6);
        position += step;
        record->Unknown19B = static_cast<uint8_t>(position >> 6);

        // shr ebx, 6 (pinned oracle model): CF = last bit out, AF = 0, OF = 0, SF = 0
        const uint32_t result = position >> 6;
        flags = ((position >> 5) & 1u) | parityFlag(result) | (result == 0u ? 0x40u : 0u);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00431b00, FUN_00431b00, "regs(eax:i32[-200..8000], esi:ptr[0x1a8]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
