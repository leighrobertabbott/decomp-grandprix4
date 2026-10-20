#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// `eax = (a - b) * weight >> 14 + b` for one component (imul, shrd 14, add); returns the sum and the add flags.
static inline uint32_t blend(int32_t target, int32_t mean, int32_t weight, uint32_t* flags) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(target) - static_cast<uint32_t>(mean)))
        * static_cast<int64_t>(weight);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t shifted = (low >> 14) | (high << 18);
    *flags = addFlags32(shifted, static_cast<uint32_t>(mean));
    return shifted + static_cast<uint32_t>(mean);
}

// Fits the sample set selected by bit ecx of the mask dwords at [0x0062a048] (sample records of 16 bytes at
// [0x0062a044], [0x0062a04c] of them): sums the selected samples with 0x4448ca (plain sums at 0x0062a270 /
// 0x0062a288 / 0x0062a27c, sums of squares at 0x0062a28c / 0x0062a290 / 0x0062a294, count at 0x0062a2b0),
// and, when any was selected, stores the means (0x0062a298 / 0x0062a29c / 0x0062a2a0) and the root-mean-square
// spreads (0x0062a2a4 / 0x0062a2a8 / 0x0062a2ac, integer root 0x40113f of the quotient << 6) and then the
// blends mean + (spread - mean) * weight >> 14 with the weights at 0x0062a2b4 + ecx * 16 into the entry
// 0x0062a5ac + ecx * 16. pushal / popal restore every register; the flags are those of the last add (or of
// the compare of the count with zero).
// FUNCTION: GP4 0x00444917
void __cdecl FUN_00444917(gp4::Regs* r) {
    const uint32_t selector = r->ecx;

    GP4_GLOBAL(uint32_t, 0x0062a270) = 0;
    GP4_GLOBAL(uint32_t, 0x0062a288) = 0;
    GP4_GLOBAL(uint32_t, 0x0062a27c) = 0;
    GP4_GLOBAL(uint32_t, 0x0062a28c) = 0;
    GP4_GLOBAL(uint32_t, 0x0062a290) = 0;
    GP4_GLOBAL(uint32_t, 0x0062a294) = 0;
    GP4_GLOBAL(uint32_t, 0x0062a2b0) = 0;

    const uint32_t samples = GP4_GLOBAL(uint32_t, 0x0062a044);
    const uint32_t* masks = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062a048)));
    const int32_t wordOffset = static_cast<int32_t>(selector) >> 5;

    uint32_t index = 0;
    do {
        if (((masks[static_cast<int32_t>(index) + wordOffset] >> (selector & 31u)) & 1u) != 0u) {
            gp4::Regs sample = *r;
            sample.st_in = 0;
            sample.st_out = 0;
            sample.ebx = samples;
            sample.esi = index;
            gp4::call_regs(0x004448ca, sample);
        }
        ++index;
    } while (index != GP4_GLOBAL(uint32_t, 0x0062a04c));

    const int32_t count = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062a2b0));
    uint32_t flags = 0x44u;
    if (count != 0) {
        GP4_GLOBAL(int32_t, 0x0062a298) = GP4_GLOBAL(int32_t, 0x0062a270) / count;
        GP4_GLOBAL(int32_t, 0x0062a29c) = GP4_GLOBAL(int32_t, 0x0062a288) / count;
        GP4_GLOBAL(int32_t, 0x0062a2a0) = GP4_GLOBAL(int32_t, 0x0062a27c) / count;

        const uint32_t sums[3] = {0x0062a28cu, 0x0062a290u, 0x0062a294u};
        const uint32_t spreads[3] = {0x0062a2a4u, 0x0062a2a8u, 0x0062a2acu};
        for (uint32_t k = 0; k != 3u; ++k) {
            const int32_t quotient = GP4_GLOBAL(int32_t, sums[k]) / count;
            const int64_t scaled = static_cast<int64_t>(quotient) * 64;
            gp4::Regs root = *r;
            root.st_in = 0;
            root.st_out = 0;
            root.eax = static_cast<uint32_t>(scaled);
            root.edx = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 32);
            gp4::call_regs(0x0040113f, root);
            GP4_GLOBAL(uint32_t, spreads[k]) = root.eax;
        }

        const uint32_t entry = selector << 4;
        const int32_t* weights = reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(0x0062a2b4u + entry));
        uint32_t* target = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(0x0062a5acu + entry));
        for (uint32_t k = 0; k != 3u; ++k) {
            target[k] = blend(GP4_GLOBAL(int32_t, spreads[k]), GP4_GLOBAL(int32_t, 0x0062a298u + k * 4u), weights[k], &flags);
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00444917, FUN_00444917, "regs(ecx:u32[0..22]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0042458b+count_0062a04c")
