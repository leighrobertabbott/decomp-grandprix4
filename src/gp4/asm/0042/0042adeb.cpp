#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit "cmp left, right".
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul; shrd eax, edx, 14: the low dword of the 64-bit product shifted right by 14.
static inline uint32_t mulShift14(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 14);
}

// High dword of the 64-bit product (edx after imul).
static inline uint32_t mulHigh(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 32);
}

// Clamps a signed weight to 0..0x4000.
static inline uint32_t clampWeight(uint32_t value) {
    if (static_cast<int32_t>(value) > 0x4000) value = 0x4000u;
    if (static_cast<int32_t>(value) < 0) value = 0u;
    return value;
}

// FUNCTION: GP4 0x0042adeb
// pushal/popal wrapper (esi = car, ebx = the other car): looks ahead [0x603ea0] bytes of track
// records from the car's segment [esi+0x1c] (lap wrap / pit end clamp), derives the two side
// weights 0x601b64 / 0x601b68 (0..0x4000) from the record's words +0x60 / +0x62 and the word
// 0x603d9c, scales one of them by [0x601b4c] when bit 5 of byte +0x51 is set, combines them with
// the pair 0x0042e494 / 0x0042e330 / 0x0042ad15 leaves in 0x601b5c / 0x601b60 and stores 0 or
// 0xff into 0x604155. Only the flags of the last compare escape.
void __cdecl FUN_0042adeb(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(s.esi));

    uint32_t seg = GP4_FIELD(uint32_t, car, 0x1c);
    if (seg >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        seg += GP4_GLOBAL(uint32_t, 0x00603ea0);
        if (seg >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            seg = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    } else {
        seg += GP4_GLOBAL(uint32_t, 0x00603ea0);
        if (seg >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            seg -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    }
    s.edi = seg;
    uint8_t* const record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(seg));

    const uint16_t offset = GP4_GLOBAL(uint16_t, 0x00603d9c);
    const int32_t left = static_cast<int16_t>(static_cast<uint16_t>(
        static_cast<uint16_t>(offset - 0xbdu) + static_cast<uint16_t>(GP4_FIELD(uint16_t, record, 0x60) + 0x50u)));
    GP4_GLOBAL(int32_t, 0x006269ac) = left;
    const int32_t right = static_cast<int16_t>(static_cast<uint16_t>(
        static_cast<uint16_t>(GP4_FIELD(uint16_t, record, 0x62) + 0x50u) - static_cast<uint16_t>(offset + 0xbdu)));
    GP4_GLOBAL(int32_t, 0x006269b0) = right;

    const int32_t steer = GP4_GLOBAL(int16_t, 0x00604404);
    const int32_t bias = static_cast<int32_t>(mulShift14(steer, GP4_GLOBAL(int32_t, 0x00601b54)));

    uint32_t weightLeft = 0u;
    s.edx = mulHigh(steer, GP4_GLOBAL(int32_t, 0x00601b54));
    const uint32_t leftExcess = static_cast<uint32_t>(left) - 0x17au;
    if (static_cast<int32_t>(leftExcess) >= 0) {
        const int32_t base = static_cast<int32_t>(leftExcess + GP4_GLOBAL(uint32_t, 0x00601b44));
        s.edx = mulHigh(base, GP4_GLOBAL(int32_t, 0x00601b48));
        weightLeft = clampWeight(mulShift14(base, GP4_GLOBAL(int32_t, 0x00601b48)) - static_cast<uint32_t>(bias));
    }
    GP4_GLOBAL(uint32_t, 0x00601b64) = weightLeft;

    uint32_t weightRight = 0u;
    s.edx = mulHigh(steer, GP4_GLOBAL(int32_t, 0x00601b54));
    const uint32_t rightExcess = static_cast<uint32_t>(right) - 0x17au;
    if (static_cast<int32_t>(rightExcess) >= 0) {
        const int32_t base = static_cast<int32_t>(rightExcess + GP4_GLOBAL(uint32_t, 0x00601b44));
        s.edx = mulHigh(base, GP4_GLOBAL(int32_t, 0x00601b48));
        weightRight = clampWeight(mulShift14(base, GP4_GLOBAL(int32_t, 0x00601b48)) + static_cast<uint32_t>(bias));
    }
    GP4_GLOBAL(uint32_t, 0x00601b68) = weightRight;
    s.ebp = static_cast<uint32_t>(bias);

    const uint8_t sides = GP4_FIELD(uint8_t, record, 0x51);
    if ((sides & 0x20u) != 0u) {
        // bit 7 xor bit 3 picks the left weight, otherwise the right one
        const bool scaleLeft = ((sides & 0x80u) != 0u) != ((sides & 8u) != 0u);
        if (scaleLeft) {
            const int32_t w = GP4_GLOBAL(int32_t, 0x00601b64);
            s.edx = mulHigh(w, GP4_GLOBAL(int32_t, 0x00601b4c));
            GP4_GLOBAL(uint32_t, 0x00601b64) = mulShift14(w, GP4_GLOBAL(int32_t, 0x00601b4c));
        } else {
            const int32_t w = GP4_GLOBAL(int32_t, 0x00601b68);
            s.edx = mulHigh(w, GP4_GLOBAL(int32_t, 0x00601b4c));
            GP4_GLOBAL(uint32_t, 0x00601b68) = mulShift14(w, GP4_GLOBAL(int32_t, 0x00601b4c));
        }
    }

    s.eax = static_cast<uint32_t>(steer);
    gp4::call_regs(0x0042e494, s);
    gp4::call_regs(0x0042e330, s);
    gp4::call_regs(0x0042ad15, s);
    GP4_GLOBAL(uint32_t, 0x00601b5c) = s.eax;
    GP4_GLOBAL(uint32_t, 0x00601b60) = s.edx;

    const int32_t gripLeft = GP4_GLOBAL(int32_t, 0x00601b5c);
    const int32_t gripRight = GP4_GLOBAL(int32_t, 0x00601b60);
    const uint32_t p = mulShift14(gripLeft, GP4_GLOBAL(int32_t, 0x00601b64));
    const uint32_t q = mulShift14(gripRight, GP4_GLOBAL(int32_t, 0x00601b68));
    uint32_t spread = q - p;
    if (static_cast<int32_t>(spread) < 0) spread = 0u - spread;
    const uint32_t scaledSpread = mulShift14(static_cast<int32_t>(spread), GP4_GLOBAL(int32_t, 0x00601b6c));
    const uint32_t total = q + p;

    uint32_t flags;
    bool clear;
    if (static_cast<int32_t>(scaledSpread) < static_cast<int32_t>(total)) {
        s.eax = GP4_GLOBAL(uint32_t, 0x00601b5c);
        s.edx = GP4_GLOBAL(uint32_t, 0x00601b60);
        gp4::call_regs(0x0042adb2, s);
        const uint32_t gripBalance = s.eax;
        s.ebp = gripBalance;
        s.eax = GP4_GLOBAL(uint32_t, 0x00601b64);
        s.edx = GP4_GLOBAL(uint32_t, 0x00601b68);
        gp4::call_regs(0x0042adb2, s);
        if (static_cast<int32_t>(s.eax) < static_cast<int32_t>(gripBalance)) {
            const uint32_t a = GP4_GLOBAL(uint32_t, 0x00601b5c);
            const uint32_t b = GP4_GLOBAL(uint32_t, 0x00601b60);
            flags = compareFlags32(a, b);
            clear = static_cast<int32_t>(a) < static_cast<int32_t>(b);
        } else {
            const uint32_t a = GP4_GLOBAL(uint32_t, 0x00601b64);
            const uint32_t b = GP4_GLOBAL(uint32_t, 0x00601b68);
            flags = compareFlags32(a, b);
            clear = static_cast<int32_t>(a) < static_cast<int32_t>(b);
        }
    } else {
        flags = compareFlags32(p, q);
        clear = static_cast<int32_t>(p) < static_cast<int32_t>(q);
    }
    GP4_GLOBAL(uint8_t, 0x00604155) = clear ? 0u : 0xffu;

    // popal: every register comes back; the flags are those of the last compare.
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042adeb, FUN_0042adeb, "regs(esi:ptr[0x420]:layout=car_004129fa+rec_00412b88, ebx:ptr[0x225]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+ring_00405a12+links_00405a12+state_0042bb90+bounds_0042e330+state_0042adeb")
