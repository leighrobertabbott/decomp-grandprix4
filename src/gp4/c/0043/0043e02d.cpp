#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of "cmp left, right" (32-bit).
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Sets bit 2 of byte +0x70 of the block at edi when |((DAT_00626964 + DAT_00626968) >> 1) *
// DAT_0062ac48| (low 32 bits, signed) reaches 0x5000. eax and edx are saved and restored. The
// flags are those of the signed "cmp eax, 0x5000" when it falls through (the value is below
// 0x5000) and those of the "or byte [edi+0x70], 4" otherwise.
// FUNCTION: GP4 0x0043e02d
void __cdecl FUN_0043e02d(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const int32_t sum = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00626964) + GP4_GLOBAL(uint32_t, 0x00626968));
    const int32_t half = sum >> 1;
    uint32_t value = static_cast<uint32_t>(half) * GP4_GLOBAL(uint32_t, 0x0062ac48);
    if (static_cast<int32_t>(value) < 0) {
        value = 0u - value;
    }

    uint32_t flags;
    if (static_cast<int32_t>(value) < 0x5000) {
        flags = compareFlags32(value, 0x5000u);
    } else {
        const uint32_t updated = GP4_FIELD(uint8_t, body, 0x70) | 4u;
        GP4_FIELD(uint8_t, body, 0x70) = static_cast<uint8_t>(updated);
        flags = parityFlag(updated) | ((updated & 0xffu) == 0u ? 0x40u : 0u) | ((updated & 0x80u) != 0u ? 0x80u : 0u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043e02d, FUN_0043e02d, "regs(edi:ptr[0x78]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
