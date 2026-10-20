#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// imul (32x32 -> edx:eax, signed) followed by "shrd eax, edx, count"; returns the shifted low dword
// and leaves the product halves in *high / *low.
static inline uint32_t mulShrd(int32_t left, int32_t right, unsigned count, uint32_t* high, uint32_t* low) {
    const int64_t product = static_cast<int64_t>(left) * static_cast<int64_t>(right);
    *low = static_cast<uint32_t>(product);
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return (*low >> count) | (*high << (32u - count));
}

// FUNCTION: GP4 0x00410154
// Per-wheel (ecx = wheel) filter coefficients written into the dword groups esi[0x3ac..0x3fc + ecx*4]
// from the setup bytes at ebx[ecx + 0x14 / 0x18 / 0x1c / 0x20]. When byte 0x00601f1f is 0xff the
// full computation runs here: the byte +0x20 (+8, at most 0x20) times DAT_00601d6c (Q14) goes to
// +0x3ac; (min(byte +0x18, 8) + 8) * 0x400 times that to +0x3bc; byte +0x1c (+8, at most 0x20)
// times DAT_00601d6c times DAT_00601e60 to +0x3cc and (min(byte +0x14, 8) + 8) * 0x400 times that to
// +0x3dc; -DAT_00601e50 times +0x3ac and -DAT_00601e54 times +0x3cc, shifted by 10, to +0x3ec /
// +0x3fc. Any other value runs 0x00410242. On the full path ebp = the +0x3cc value, eax / edx =
// the last product (shifted / high dword) and the flags are those of the last shrd.
void __cdecl FUN_00410154(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x00601f1f) != 0xffu) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x00410242, c);
        r->eax = c.eax;
        r->edx = c.edx;
        r->ebp = c.ebp;
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
        return;
    }

    const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t index = r->ecx;

    uint32_t high;
    uint32_t low;

    int32_t sample = static_cast<int32_t>(source[index + 0x20u]) + 8;
    if (sample > 0x20) {
        sample = 0x20;
    }
    const uint32_t first = mulShrd(sample, GP4_GLOBAL(int32_t, 0x00601d6c), 14, &high, &low);
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3acu) = first;

    int32_t level = static_cast<int32_t>(source[index + 0x18u]);
    if (level > 8) {
        level = 8;
    }
    const uint32_t second = mulShrd((level + 8) << 10, static_cast<int32_t>(first), 14, &high, &low);
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3bcu) = second;

    sample = static_cast<int32_t>(source[index + 0x1cu]) + 8;
    if (sample > 0x20) {
        sample = 0x20;
    }
    const uint32_t third0 = mulShrd(sample, GP4_GLOBAL(int32_t, 0x00601d6c), 14, &high, &low);
    const uint32_t third = mulShrd(static_cast<int32_t>(third0), GP4_GLOBAL(int32_t, 0x00601e60), 14, &high, &low);
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3ccu) = third;

    level = static_cast<int32_t>(source[index + 0x14u]);
    if (level > 8) {
        level = 8;
    }
    const uint32_t fourth = mulShrd((level + 8) << 10, static_cast<int32_t>(third), 14, &high, &low);
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3dcu) = fourth;

    const int32_t negA = static_cast<int32_t>(0u - GP4_GLOBAL(uint32_t, 0x00601e50));
    const uint32_t fifth = mulShrd(negA, GP4_FIELD(int32_t, dest, index * 4u + 0x3acu), 10, &high, &low);
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3ecu) = fifth;

    const int32_t negB = static_cast<int32_t>(0u - GP4_GLOBAL(uint32_t, 0x00601e54));
    const uint32_t sixth = mulShrd(negB, GP4_FIELD(int32_t, dest, index * 4u + 0x3ccu), 10, &high, &low);
    GP4_FIELD(uint32_t, dest, index * 4u + 0x3fcu) = sixth;

    // Flags of the final shrd by 10 (pinned oracle model): CF = last bit out, AF = 0,
    // OF = bit31(shift by 9 ^ shift by 10).
    const uint32_t previous = (low >> 9) | (high << 23);
    const uint32_t flags = ((low >> 9) & 1u)
        | parityFlag(sixth)
        | (sixth == 0u ? 0x40u : 0u)
        | ((sixth & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ sixth) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = sixth;
    r->edx = high;
    r->ebp = third;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00410154, FUN_00410154, "regs(ebx:ptr[0x40]:layout=setup_00410a1c, ecx:u32[0..3], esi:ptr[0x410]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=wheel_00410a1c")
