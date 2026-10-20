#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// imul (32x32 -> edx:eax, signed) followed by "shrd eax, edx, count"; returns the shifted low dword
// and leaves the high dword in *high.
static inline uint32_t mulShrd(int32_t left, int32_t right, unsigned count, uint32_t* high, uint32_t* low) {
    const int64_t product = static_cast<int64_t>(left) * static_cast<int64_t>(right);
    *low = static_cast<uint32_t>(product);
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return (*low >> count) | (*high << (32u - count));
}

// min(byte + 8, 0x20) read from source[index + 0x20]
static inline int32_t clampedSample(const uint8_t* source, uint32_t index) {
    int32_t value = static_cast<int32_t>(source[index + 0x20u]) + 8;
    if (value > 0x20) {
        value = 0x20;
    }
    return value;
}

// FUNCTION: GP4 0x00410242
// Per-index (ecx) filter coefficients written to the dword slots esi[0x3ac..0x3fc + ecx*4], four-dword
// (0x10 byte) groups apart. The Q14 product of the clamped byte at ebx[ecx+0x20] and DAT_00601d6c goes
// to groups 0x3ac and 0x3bc; that times DAT_00601e60 (Q14) to 0x3cc and 0x3dc; -DAT_00601e50 times the
// first and -DAT_00601e54 times the third are shifted by 10 into 0x3ec and 0x3fc. ebp = the value of the
// 0x3cc group, eax/edx = the last product (shifted / high dword); the flags are those of the last shrd.
void __cdecl FUN_00410242(gp4::Regs* r) {
    const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t index = r->ecx;

    uint32_t high;
    uint32_t low;

    const uint32_t first = mulShrd(clampedSample(source, index), GP4_GLOBAL(int32_t, 0x00601d6c), 14, &high, &low);
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3acu) = first;
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3bcu) = first;

    const uint32_t second0 = mulShrd(clampedSample(source, index), GP4_GLOBAL(int32_t, 0x00601d6c), 14, &high, &low);
    const uint32_t second =
        mulShrd(static_cast<int32_t>(second0), GP4_GLOBAL(int32_t, 0x00601e60), 14, &high, &low);
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3ccu) = second;
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3dcu) = second;

    const int32_t negA = static_cast<int32_t>(0u - GP4_GLOBAL(uint32_t, 0x00601e50));
    const uint32_t third =
        mulShrd(negA, GP4_FIELD(int32_t, dest, index * 4u + 0x3acu), 10, &high, &low);
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3ecu) = third;

    const int32_t negB = static_cast<int32_t>(0u - GP4_GLOBAL(uint32_t, 0x00601e54));
    const uint32_t fourth =
        mulShrd(negB, GP4_FIELD(int32_t, dest, index * 4u + 0x3ccu), 10, &high, &low);
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3fcu) = fourth;

    // Flags of the final shrd by 10 (pinned oracle model): CF = last bit out, AF = 0,
    // OF = bit31(shift by 9 ^ shift by 10).
    const uint32_t previous = (low >> 9) | (high << 23);
    const uint32_t flags = ((low >> 9) & 1u)
        | parityFlag(fourth)
        | (fourth == 0u ? 0x40u : 0u)
        | ((fourth & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ fourth) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = fourth;
    r->edx = high;
    r->ebp = second;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00410242, FUN_00410242, "regs(ebx:ptr[0x40]:bytes, ecx:u32[0..15], esi:ptr[0x448]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
