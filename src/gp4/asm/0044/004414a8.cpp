#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `shl value, 2` under the oracle model: CF = last bit shifted out, AF = 0,
// OF = bit31((value << 1) ^ result).
static inline uint32_t shl2Flags32(uint32_t value) {
    const uint32_t result = value << 2;
    return ((value >> 30) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((value << 1) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_Car {
    uint8_t Unknown00[0x7c];
    uint8_t Number7C;
    uint8_t Unknown7D[0x188 - 0x7d];
    uint32_t Share188;
    uint32_t Share18C;
    uint32_t Share190;
    uint32_t Share194;
};

// Row of the per-car tables for a car number byte: (number & 0x3f) - 1, signed, at most 0x15.
static inline int32_t tableRow(uint8_t number) {
    const int32_t row = static_cast<int32_t>(number & 0x3fu) - 1;
    return row > 0x15 ? 0x15 : row;
}

// 240 * value / total, clamped to 0xf0 (signed compare).
static inline uint8_t scaledShare(uint32_t value, uint32_t total) {
    uint32_t remainder;
    uint32_t scaled = gp4::div64(static_cast<uint64_t>(value) * 0xf0u, total, &remainder);
    if (static_cast<int32_t>(scaled) > 0xf0) scaled = 0xf0u;
    return static_cast<uint8_t>(scaled);
}

// pushal / popal: writes the four shares (value 240 * share / largest of the four, one byte each) of the car
// record in esi into four byte tables (row stride 0x2c, column = byte [row + 0x0066d936]), and the largest
// share scaled by the signed factor [0x00602a9c] >> 14 into the matching dword table at 0x0066ec60. Registers
// are restored; the flags are those of the final `shl eax, 2` on row * 0x2c.
// FUNCTION: GP4 0x004414a8
void __cdecl FUN_004414a8(gp4::Regs* r) {
    Unknown_Car* car = reinterpret_cast<Unknown_Car*>(static_cast<uintptr_t>(r->esi));

    const int32_t row = tableRow(car->Number7C);
    const uint32_t rowOffset = static_cast<uint32_t>(row * 0x2c);
    const uint32_t column = GP4_ARRAY(const uint8_t, 0x0066d936)[row];

    uint32_t largest = car->Share188;
    if (largest < car->Share18C) largest = car->Share18C;
    if (largest < car->Share190) largest = car->Share190;
    if (largest < car->Share194) largest = car->Share194;
    const uint32_t total = largest == 0u ? 1u : largest;

    GP4_ARRAY(uint8_t, 0x0066dd2a)[rowOffset + column] = scaledShare(car->Share188, total);
    GP4_ARRAY(uint8_t, 0x0066e0f2)[rowOffset + column] = scaledShare(car->Share18C, total);
    GP4_ARRAY(uint8_t, 0x0066e4ba)[rowOffset + column] = scaledShare(car->Share190, total);
    GP4_ARRAY(uint8_t, 0x0066e882)[rowOffset + column] = scaledShare(car->Share194, total);

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(total)) *
                            static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00602a9c));
    const uint32_t scaledTotal = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    const int32_t row2 = tableRow(car->Number7C);
    const uint32_t column2 = GP4_ARRAY(const uint8_t, 0x0066d936)[row2];
    const uint32_t rowOffset2 = static_cast<uint32_t>(row2 * 0x2c);
    GP4_ARRAY(uint32_t, 0x0066ec60)[rowOffset2 + column2] = scaledTotal;

    r->eflags = (r->eflags & ~0x8d5u) | shl2Flags32(rowOffset2);
}
GP4_IMPL(0x004414a8, FUN_004414a8, "regs(esi:ptr[0x19c]:layout=rec_004414a8) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
