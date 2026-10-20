#include <gp4/gp4.h>

static inline uint32_t ParityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (~parity & 1u) << 2;
}

static inline uint32_t SubtractFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return (lhs < rhs ? 1u : 0u) | ParityFlag(result)
        | ((lhs ^ rhs ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// Flags left by or/xor/test: CF = OF = AF = 0.
static inline uint32_t LogicFlags(uint32_t result) {
    return ParityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7);
}

// Flags left by "shrd eax, edx, 14" on the 64-bit product edx:eax (pinned oracle model).
static inline uint32_t Shrd14Flags(uint64_t product, uint32_t result) {
    const uint32_t previous = static_cast<uint32_t>(product >> 13);
    return static_cast<uint32_t>((product >> 13) & 1u) | ParityFlag(result)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | (((previous ^ result) >> 31) << 11);
}

// FUNCTION: GP4 0x00416dec
void __cdecl FUN_00416dec(gp4::Regs* r) {
    uint8_t* state = GP4_ARRAY(uint8_t, 0x006380d8);

    int32_t gate = GP4_GLOBAL(int32_t, 0x00638d30);
    if (gate < GP4_GLOBAL(int32_t, 0x0062fbec)) gate = GP4_GLOBAL(int32_t, 0x0062fbec);

    uint32_t lowOut = GP4_GLOBAL(uint32_t, 0x00637f38);
    uint32_t highOut = GP4_GLOBAL(uint32_t, 0x00637f3c);
    uint32_t flags = 0;
    bool stateLoaded = false;
    bool blended = false;
    int32_t weight = 0;

    do {
        if (gate == 0) {
            flags = LogicFlags(0);
            break;
        }
        int32_t magnitude = GP4_GLOBAL(int32_t, 0x00638ce0);
        if (magnitude < 0) magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(magnitude));
        const int32_t threshold = GP4_GLOBAL(int32_t, 0x0062fb9c);
        if (magnitude < threshold) {
            flags = SubtractFlags(static_cast<uint32_t>(magnitude), static_cast<uint32_t>(threshold),
                                  static_cast<uint32_t>(magnitude) - static_cast<uint32_t>(threshold));
            break;
        }

        stateLoaded = true;
        const int32_t first = static_cast<int32_t>(GP4_FIELD(uint32_t, state, 0x640) + GP4_FIELD(uint32_t, state, 0x940)) >> 1;
        const int32_t second = static_cast<int32_t>(GP4_FIELD(uint32_t, state, 0x61c) + GP4_FIELD(uint32_t, state, 0x91c)) >> 1;
        const uint32_t signs = static_cast<uint32_t>(second) ^ static_cast<uint32_t>(first);
        if (static_cast<int32_t>(signs) >= 0) {
            flags = LogicFlags(signs);
            break;
        }

        int32_t level = static_cast<int32_t>(GP4_FIELD(uint32_t, state, 0x628) + GP4_FIELD(uint32_t, state, 0x928)) >> 1;
        if (level < 0) level = static_cast<int32_t>(0u - static_cast<uint32_t>(level));
        const int32_t floorLevel = GP4_GLOBAL(int32_t, 0x0062fba0);
        if (level < floorLevel) {
            flags = SubtractFlags(static_cast<uint32_t>(level), static_cast<uint32_t>(floorLevel),
                                  static_cast<uint32_t>(level) - static_cast<uint32_t>(floorLevel));
            break;
        }

        const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(level) - static_cast<uint32_t>(floorLevel)))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062fba4));
        int32_t t = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14));
        if (t > 0x4000) t = 0x4000;
        if (t < 0) t = 0;
        weight = 0x4000 - t;
        GP4_GLOBAL(int32_t, 0x0062fb98) = weight;
        blended = true;

        const int64_t highProduct = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00637f3c)) * static_cast<int64_t>(weight);
        highOut = static_cast<uint32_t>(static_cast<uint64_t>(highProduct) >> 14);
        const int64_t lowProduct = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00637f38)) * static_cast<int64_t>(weight);
        lowOut = static_cast<uint32_t>(static_cast<uint64_t>(lowProduct) >> 14);
        flags = Shrd14Flags(static_cast<uint64_t>(lowProduct), lowOut);
    } while (false);

    GP4_GLOBAL(uint32_t, 0x0062fba8) = lowOut;
    GP4_GLOBAL(uint32_t, 0x0062fbac) = highOut;

    r->eax = lowOut;
    r->edx = highOut;
    if (stateLoaded) r->ebx = 0x006380d8u;
    if (blended) r->ebp = static_cast<uint32_t>(weight);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00416dec, FUN_00416dec, "regs(ebx:u32, ebp:u32) -> (eax, edx, ebx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00416dec")
