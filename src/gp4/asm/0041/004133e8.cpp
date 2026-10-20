#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by TEST/AND/OR on a value of the given width (CF = OF = AF = 0).
static inline uint32_t logicFlags(uint32_t result, uint32_t bits) {
    const uint32_t mask = bits == 32u ? 0xffffffffu : ((1u << bits) - 1u);
    return parityFlag8(result)
        | ((result & mask) == 0u ? 0x40u : 0u)
        | (((result >> (bits - 1u)) & 1u) != 0u ? 0x80u : 0u);
}

// Flags of `a - b` (cmp/sub) on a value of the given width.
static inline uint32_t subFlags(uint32_t a, uint32_t b, uint32_t bits) {
    const uint32_t mask = bits == 32u ? 0xffffffffu : ((1u << bits) - 1u);
    const uint32_t res = (a - b) & mask;
    a &= mask;
    b &= mask;
    return (a < b ? 1u : 0u)
        | parityFlag8(res)
        | (((a ^ b ^ res) & 0x10u) != 0u ? 0x10u : 0u)
        | (res == 0u ? 0x40u : 0u)
        | (((res >> (bits - 1u)) & 1u) != 0u ? 0x80u : 0u)
        | (((((a ^ b) & (a ^ res)) >> (bits - 1u)) & 1u) != 0u ? 0x800u : 0u);
}

static inline uint32_t withAx(uint32_t eax, uint32_t ax) {
    return (eax & 0xffff0000u) | (ax & 0xffffu);
}

// Per-car record (only the fields this routine reads; offsets in the names).
struct Unknown_004133e8_Car {
    uint8_t Unknown00[0x0e];
    int16_t Unknown0E;
    uint8_t Unknown10[0x78 - 0x10];
    uint8_t Flags78;
    uint8_t Unknown79[0x90 - 0x79];
    int16_t Unknown90;
    uint8_t Unknown92[0x9a - 0x92];
    int16_t Unknown9A;
    uint8_t Unknown9C[0xb0 - 0x9c];
    int16_t UnknownB0;
    uint8_t UnknownB2[0xe4 - 0xb2];
    int8_t UnknownE4;
    uint8_t FlagsE5;
    uint8_t UnknownE6[0x142 - 0xe6];
    int16_t Unknown142;
    uint8_t Unknown144[0x162 - 0x144];
    uint8_t Flags162;
    uint8_t Unknown163[0x1ce - 0x163];
    uint8_t Flags1CE;
    uint8_t Unknown1CF[0x232 - 0x1cf];
    uint8_t Flags232;
};

struct Outcome {
    uint32_t eax;
    uint32_t edx;
    uint32_t flags;
    bool set;
};

static Outcome Done(uint32_t eax, uint32_t edx, uint32_t flags, bool set) {
    Outcome o;
    o.eax = eax;
    o.edx = edx;
    o.flags = flags;
    o.set = set;
    return o;
}

static Outcome Evaluate(const Unknown_004133e8_Car* car, uint32_t eax, uint32_t edx) {
    if (car->UnknownE4 >= 2 && car->UnknownE4 <= 5) {
        return Done(eax, edx, subFlags(static_cast<uint8_t>(car->UnknownE4), 5u, 8u), false);
    }

    eax = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00638ce0) >> 8);
    const uint16_t floorValue = GP4_GLOBAL(uint16_t, 0x006013e0);
    if (static_cast<int16_t>(eax) < static_cast<int16_t>(floorValue)) {
        return Done(eax, edx, subFlags(eax, floorValue, 16u), false);
    }

    if ((car->Flags232 & 1u) != 0u) return Done(eax, edx, logicFlags(car->Flags232 & 1u, 8u), true);
    if ((car->FlagsE5 & 4u) != 0u) return Done(eax, edx, logicFlags(car->FlagsE5 & 4u, 8u), true);
    if ((car->Flags78 & 0x20u) == 0u) return Done(eax, edx, logicFlags(0u, 8u), false);
    if ((car->Flags162 & 8u) != 0u) return Done(eax, edx, logicFlags(car->Flags162 & 8u, 8u), true);

    const uint32_t speed = GP4_GLOBAL(uint32_t, 0x00638ce0);
    if (static_cast<int32_t>(speed) < 0) return Done(eax, edx, logicFlags(speed, 32u), false);
    if ((car->Flags1CE & 1u) != 0u) return Done(eax, edx, logicFlags(car->Flags1CE & 1u, 8u), true);

    uint32_t ax = static_cast<uint16_t>(car->Unknown90);
    eax = withAx(eax, ax);
    if (static_cast<int16_t>(ax) < 0) ax = (0u - ax) & 0xffffu;
    ax = (ax - 0x400u) & 0xffffu;
    eax = withAx(eax, ax);
    const uint16_t limit = static_cast<uint16_t>(car->UnknownB0);
    if (static_cast<int16_t>(ax) >= static_cast<int16_t>(limit)) {
        edx = static_cast<uint32_t>(static_cast<int32_t>(car->Unknown9A));
        const int32_t threshold = GP4_GLOBAL(int32_t, 0x00601ef8);
        return Done(eax, edx, subFlags(edx, static_cast<uint32_t>(threshold), 32u),
                    static_cast<int32_t>(edx) >= threshold);
    }

    ax = static_cast<uint16_t>(car->Unknown0E);
    ax = (ax - static_cast<uint16_t>(car->Unknown142)) & 0xffffu;
    if (static_cast<int16_t>(ax) < 0) ax = (0u - ax) & 0xffffu;
    if (static_cast<int16_t>(ax) > 0x4000) ax = ((0u - ax) + 0x8000u) & 0xffffu;
    eax = withAx(eax, ax);
    const uint16_t bound = GP4_GLOBAL(uint16_t, 0x00603574);
    return Done(eax, edx, subFlags(ax, bound, 16u), static_cast<int16_t>(ax) < static_cast<int16_t>(bound));
}

// FUNCTION: GP4 0x004133e8
// Sets the byte DAT_006036d7 to 0xff when a car qualifies (state byte +0xe4 outside 2..5, the
// forward-speed word of DAT_00638ce0 >> 8 not below DAT_006013e0, and then one of: byte +0x232 bit 0,
// byte +0xe5 bit 2, byte +0x162 bit 3, byte +0x1ce bit 0, a large |word +0x90| with word +0x9a >=
// DAT_00601ef8, or a small angular difference of words +0xe / +0x142 below DAT_00603574; the
// forward-speed sign and byte +0x78 bit 5 can veto it); otherwise it is 0. eax, edx and the flags are
// those of the last instructions executed on each path.
void __cdecl FUN_004133e8(gp4::Regs* r) {
    const Unknown_004133e8_Car* car = reinterpret_cast<const Unknown_004133e8_Car*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint8_t, 0x006036d7) = 0;
    const Outcome o = Evaluate(car, r->eax, r->edx);
    if (o.set) {
        GP4_GLOBAL(uint8_t, 0x006036d7) = 0xff;
    }

    r->eax = o.eax;
    r->edx = o.edx;
    r->eflags = (r->eflags & ~0x8d5u) | o.flags;
}
GP4_IMPL(0x004133e8, FUN_004133e8, "regs(eax:u32, edx:u32, esi:ptr[0x23a]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
