#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (PF AF ZF SF OF, and CF) of the 8-bit `cmp left, right`.
static inline uint32_t compareFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

struct Unknown_00415ef6_Record {
    uint8_t Unknown00[0x62];
    int16_t Unknown62;
    uint8_t Unknown64[0x72 - 0x64];
    int8_t Unknown72;
    uint8_t Unknown73[0x7c - 0x73];
    uint8_t Unknown7C;
    uint8_t Unknown7D[0x9a - 0x7d];
    int16_t Unknown9A;
    uint8_t Unknown9C;
    uint8_t Unknown9D[0x1cd - 0x9d];
    int8_t Unknown1CD;
};

// FUNCTION: GP4 0x00415ef6
// Steps the signed byte at esi+0x72 up (when word[esi+0x62] >= DAT_0060131c, unless it is already
// >= 1 and word[esi+0x9a] < 0x1255; the new value is clamped to the byte at +0x1cd) or down (when
// it is below DAT_0060131c and the byte is >= 2 and word[esi+0x62] is below the word of the table at
// 0x007c28bc indexed by ((byte[+0x7c] & 0x3f) - 1, clamped to 0x15) * 16 + byte * 2; the new value
// is at least 1 and bit 2 of +0x9c is set). Returns eax = ecx as it was with its low byte replaced
// by the old byte, ecx/edx as left by the code, CF = 1 when the byte changed, and the other flags
// of `cmp al, byte [esi+0x72]`.
void __cdecl FUN_00415ef6(gp4::Regs* r) {
    Unknown_00415ef6_Record* record =
        reinterpret_cast<Unknown_00415ef6_Record*>(static_cast<uintptr_t>(r->esi));
    const uint8_t oldValue = static_cast<uint8_t>(record->Unknown72);
    uint32_t ecx = (r->ecx & 0xffffff00u) | oldValue;
    uint32_t edx = r->edx;
    const uint32_t pushed = ecx;
    const int32_t position = record->Unknown62;

    if (position >= GP4_GLOBAL(int32_t, 0x0060131c)) {
        const bool unchanged = static_cast<int8_t>(oldValue) >= 1 && record->Unknown9A < 0x1255;
        if (!unchanged) {
            uint8_t value = static_cast<uint8_t>(oldValue + 1u);
            if (static_cast<int8_t>(value) > record->Unknown1CD) {
                value = static_cast<uint8_t>(record->Unknown1CD);
            }
            ecx = (ecx & 0xffffff00u) | value;
            record->Unknown72 = static_cast<int8_t>(value);
        }
    } else if (static_cast<int8_t>(oldValue) >= 2) {
        int32_t row = static_cast<int32_t>(record->Unknown7C & 0x3fu) - 1;
        if (row > 0x15) {
            row = 0x15;
        }
        edx = static_cast<uint32_t>(row) << 4;
        ecx = oldValue;
        const int16_t limit = GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c28bc), edx + ecx * 2u);
        if (static_cast<int16_t>(position) < limit) {
            const uint8_t high = oldValue;
            uint8_t value = static_cast<uint8_t>(oldValue - 1u);
            if (static_cast<int8_t>(value) < 1) {
                value = 1;
            }
            ecx = (static_cast<uint32_t>(high) << 8) | value;
            if (static_cast<int8_t>(high) >= 2) {
                record->Unknown9C = static_cast<uint8_t>(record->Unknown9C | 4u);
            }
            record->Unknown72 = static_cast<int8_t>(value);
        }
    }

    uint32_t flags = compareFlags8(pushed, static_cast<uint8_t>(record->Unknown72));
    flags = (flags & ~1u) | ((flags & 0x40u) == 0u ? 1u : 0u);
    r->eax = pushed;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00415ef6, FUN_00415ef6, "regs(esi:ptr[0x1d5]:bytes, ecx:u32, edx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
