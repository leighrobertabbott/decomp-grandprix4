#include <gp4/gp4.h>

// Fills eight dwords of the esi record from the four 0x300-byte records at 0x6380d8. scale is the
// sum of dwords +0x3c/+0x33c/+0x63c/+0x93c times [0x6013b8] >> 14. The value for the first group
// (+0x2bc/+0x2c0/+0x2ec/+0x2f0) is (word +0xb2 * dword 0x638270 >> 6) + ((scale - [0x637fb4]) >> 1)
// and for the second group (+0x2c4/+0x2c8/+0x2f4/+0x2f8) (word +0xb2 * dword 0x638870 >> 6) +
// ((-[0x637fc0] - scale) >> 1); both clamp at 0. Leaves ebx = 0x6380d8, ecx = scale, edx = the last
// halved difference and eax = the second clamped value; the flags are those of the second CMP with 0.
static inline uint32_t parity_flag(uint32_t value) {
    uint32_t b = value & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return (b & 1u) ? 0u : 4u;   // PF set when the low byte has an even number of set bits
}

// FUNCTION: GP4 0x00419666
void __cdecl FUN_00419666(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* bodies = GP4_ARRAY(uint8_t, 0x006380d8);

    const uint32_t total = GP4_FIELD(uint32_t, bodies, 0x3c) + GP4_FIELD(uint32_t, bodies, 0x33c)
        + GP4_FIELD(uint32_t, bodies, 0x63c) + GP4_FIELD(uint32_t, bodies, 0x93c);
    const int64_t weighted = static_cast<int64_t>(static_cast<int32_t>(total)) * GP4_GLOBAL(int32_t, 0x006013b8);
    const uint32_t scale = static_cast<uint32_t>(static_cast<uint64_t>(weighted) >> 14);

    const uint32_t width = GP4_FIELD(uint16_t, rec, 0xb2);

    const int64_t first_product = static_cast<int64_t>(width) * GP4_FIELD(int32_t, bodies, 0x198);
    const uint32_t first_half = static_cast<uint32_t>((0u - GP4_GLOBAL(uint32_t, 0x00637fb4)) + scale);
    int32_t first = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(first_product) >> 6)
        + static_cast<uint32_t>(static_cast<int32_t>(first_half) >> 1));
    if (first < 0) first = 0;
    GP4_FIELD(int32_t, rec, 0x2ec) = first;
    GP4_FIELD(int32_t, rec, 0x2bc) = first;
    GP4_FIELD(int32_t, rec, 0x2f0) = first;
    GP4_FIELD(int32_t, rec, 0x2c0) = first;

    const int64_t second_product = static_cast<int64_t>(width) * GP4_FIELD(int32_t, bodies, 0x798);
    const uint32_t second_diff = static_cast<uint32_t>((0u - GP4_GLOBAL(uint32_t, 0x00637fc0)) - scale);
    const uint32_t second_half = static_cast<uint32_t>(static_cast<int32_t>(second_diff) >> 1);
    const uint32_t second_raw = static_cast<uint32_t>(static_cast<uint64_t>(second_product) >> 6) + second_half;
    int32_t second = static_cast<int32_t>(second_raw);
    if (second < 0) second = 0;
    GP4_FIELD(int32_t, rec, 0x2f4) = second;
    GP4_FIELD(int32_t, rec, 0x2c4) = second;
    GP4_FIELD(int32_t, rec, 0x2f8) = second;
    GP4_FIELD(int32_t, rec, 0x2c8) = second;

    // CMP second_raw, 0: CF = OF = AF = 0; ZF, SF and PF from the value.
    const uint32_t flags = parity_flag(second_raw)
        | (second_raw == 0u ? 0x40u : 0u)
        | ((second_raw & 0x80000000u) ? 0x80u : 0u);

    r->eax = static_cast<uint32_t>(second);
    r->ebx = 0x006380d8u;
    r->ecx = scale;
    r->edx = second_half;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00419666, FUN_00419666, "regs(esi:ptr[0x300]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
