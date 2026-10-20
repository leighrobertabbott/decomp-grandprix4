#include <gp4/gp4.h>

// Only the fields this routine reads or writes: four per-slot dwords at each of these offsets.
struct Unknown_004105c7 {
    uint8_t Unknown000[0x2cc];
    uint32_t Unknown2cc[4];
    uint32_t Unknown2dc[4];
    uint8_t Unknown2ec[0x30];
    uint32_t Unknown31c[4];
    uint32_t Unknown32c[4];
    uint32_t Unknown33c[4];
};
static_assert(sizeof(Unknown_004105c7) == 0x34c, "layout");

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subtractFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// ecx = slot (0..3), esi = record. For slot i: value = DAT_00601d7c[i];
//   [+0x31c + 4i] = (value << 10) / DAT_00601d9c[i]
//   [+0x32c + 4i] = ((value << 6) / [+0x2cc + 4i]) << 4
//   [+0x33c + 4i] = DAT_00601df4[i] - [+0x32c + 4i] - [+0x31c + 4i] - [+0x2dc + 4i]
// Leaves eax = the last result, edx = the remainder of the second division and the flags of the
// final subtraction.
// FUNCTION: GP4 0x004105c7
void __cdecl FUN_004105c7(gp4::Regs* r) {
    Unknown_004105c7* record = reinterpret_cast<Unknown_004105c7*>(static_cast<uintptr_t>(r->esi));
    const uint32_t slot = r->ecx;

    uint32_t remainder = 0;
    const uint32_t first = GP4_ARRAY(uint32_t, 0x00601d7c)[slot];
    record->Unknown31c[slot] = gp4::div64(static_cast<uint64_t>(first) << 10, GP4_ARRAY(uint32_t, 0x00601d9c)[slot], &remainder);

    const uint32_t second = GP4_ARRAY(uint32_t, 0x00601d7c)[slot];
    const uint32_t quotient = gp4::div64(static_cast<uint64_t>(second) << 6, record->Unknown2cc[slot], &remainder);
    record->Unknown32c[slot] = quotient << 4;

    const uint32_t start = GP4_ARRAY(uint32_t, 0x00601df4)[slot];
    const uint32_t step1 = start - record->Unknown32c[slot];
    const uint32_t step2 = step1 - record->Unknown31c[slot];
    const uint32_t divisor3 = record->Unknown2dc[slot];
    const uint32_t result = step2 - divisor3;
    record->Unknown33c[slot] = result;

    r->eax = result;
    r->edx = remainder;
    r->eflags = (r->eflags & ~0x8d5u) | subtractFlags32(step2, divisor3);
}
GP4_IMPL(0x004105c7, FUN_004105c7, "regs(ecx:u32[0..3], esi:ptr[0x350]:layout=rec_004105c7) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_004105c7")
