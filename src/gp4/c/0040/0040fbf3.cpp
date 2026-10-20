#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// imul (one operand) then "shrd eax, edx, 14": bits 14..45 of the signed 64-bit product.
static inline uint32_t mulShift14(uint32_t a, uint32_t b, uint32_t* high) {
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(a))
                                                   * static_cast<int64_t>(static_cast<int32_t>(b)));
    *high = static_cast<uint32_t>(product >> 32);
    return static_cast<uint32_t>(product >> 14);
}

// The setup override of the code bytes: active when [0x601f20] != 0xff and the mask byte [0x601fab]
// hits the low byte of the dword 0x601f28[[0x601fa8] & 3] (& 1 when [0x601fa9] != 0).
static inline bool overrideActive() {
    if (GP4_GLOBAL(uint8_t, 0x00601f20) == 0xffu) return false;
    uint32_t index = GP4_GLOBAL(uint8_t, 0x00601fa8) & 3u;
    if (GP4_GLOBAL(uint8_t, 0x00601fa9) != 0u) index &= 1u;
    const uint32_t mask = GP4_ARRAY(uint32_t, 0x00601f28)[index] & 0xffu;
    return (GP4_GLOBAL(uint8_t, 0x00601fab) & mask) != 0u;
}

// Setup modes 1 and 3 take the second replacement byte.
static inline bool alternateMode() {
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x00601fa8);
    return mode == 1u || mode == 3u;
}

// Code byte +1 of the car's data entry, or its replacement 0x62d1b9 / 0x62d1c9.
static inline uint32_t codeSecond(const uint8_t* data) {
    uint32_t code = data[1];
    if (overrideActive()) {
        code = alternateMode() ? GP4_GLOBAL(uint8_t, 0x0062d1c9) : GP4_GLOBAL(uint8_t, 0x0062d1b9);
    }
    return code;
}

// Code byte +0 of the car's data entry, or its replacement 0x62d1b8 / 0x62d1c8.
static inline uint32_t codeFirst(const uint8_t* data) {
    uint32_t code = data[0];
    if (overrideActive()) {
        code = alternateMode() ? GP4_GLOBAL(uint8_t, 0x0062d1c8) : GP4_GLOBAL(uint8_t, 0x0062d1b8);
    }
    return code;
}

// FUNCTION: GP4 0x0040fbf3
// pushal/popal around: selects the car's data entry (0x40fb09 -> [0x601f24]), stores the two code
// bytes (+0x227, +0x26a), the 0x40fb47 curve value scaled by [0x602fe0] (+0x164, low word also +0xb8),
// round((((b0 - h) * [0x62ec68] >> 14) + h - b1) * [0x6012d0] + [0x62ec78]) * [0x6f2b04]) with
// h = [0x62ec6c] >> 1 (+0x16a, then rescaled by 0x410047), the 0x40fb95 ratio scaled by [0x602fe0]
// (+0x16c) and ((+0x16c << 13) / [0x60138c] * [0x603854]) >> 16 (+0xc4). Every register comes
// back; the flags are those of the final shrd.
void __cdecl FUN_0040fbf3(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    gp4::call_regs(0x0040fb09, s);

    const uint8_t* const data = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00601f24)));
    s.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data));

    const uint32_t second = codeSecond(data);
    s.eax = (s.eax & ~0xffu) | second;
    GP4_FIELD(uint8_t, car, 0x227) = static_cast<uint8_t>(second);
    gp4::call_regs(0x0040fb47, s);

    uint32_t high;
    const uint32_t curve = mulShift14(s.eax, GP4_GLOBAL(uint32_t, 0x00602fe0), &high);
    GP4_FIELD(uint32_t, car, 0x164) = curve;

    const uint32_t half = GP4_GLOBAL(uint32_t, 0x0062ec6c) >> 1;
    s.ebp = half;

    const uint32_t first = codeFirst(data);
    GP4_FIELD(uint8_t, car, 0x26a) = static_cast<uint8_t>(first);
    const uint32_t centred = mulShift14(first - half, GP4_GLOBAL(uint32_t, 0x0062ec68), &high) + half;

    const uint32_t other = codeSecond(data);
    s.edx = other;
    const int32_t level = static_cast<int32_t>(centred - other);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = level;

    gp4::x87::Ext x = gp4::x87::ext_i32(level) * gp4::x87::ext(GP4_GLOBAL(double, 0x006012d0));
    x = x + gp4::x87::ext(GP4_GLOBAL(double, 0x0062ec78));
    x = x * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b04));
    const int32_t rounded = gp4::x87::fistp32(x);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = rounded;
    s.eax = static_cast<uint32_t>(rounded);
    GP4_FIELD(uint16_t, car, 0x16a) = static_cast<uint16_t>(rounded);

    gp4::call_regs(0x00410047, s);

    s.eax = GP4_FIELD(uint32_t, car, 0x164);
    GP4_FIELD(uint16_t, car, 0xb8) = static_cast<uint16_t>(s.eax);

    const uint32_t third = codeSecond(data);
    s.eax = (s.eax & ~0xffu) | third;
    gp4::call_regs(0x0040fb95, s);

    const uint32_t ratio = mulShift14(s.eax, GP4_GLOBAL(uint32_t, 0x00602fe0), &high);
    GP4_FIELD(uint32_t, car, 0x16c) = ratio;

    uint32_t remainder;
    const uint32_t quotient = gp4::div64(static_cast<uint64_t>(ratio) << 13, GP4_GLOBAL(uint32_t, 0x0060138c), &remainder);
    const uint64_t product = static_cast<uint64_t>(quotient) * static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x00603854));
    const uint32_t result = static_cast<uint32_t>(product >> 16);
    const uint32_t previous = static_cast<uint32_t>(product >> 15);
    GP4_FIELD(uint16_t, car, 0xc4) = static_cast<uint16_t>(result);

    // shrd eax, edx, 16 (oracle model: CF = bit 15 of eax, AF = 0, OF = bit31 of shift-15 ^ shift-16)
    const uint32_t flags = ((static_cast<uint32_t>(product) >> 15) & 1u) | parityFlag(result)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040fbf3, FUN_0040fbf3, "regs(esi:ptr[0x278]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=trk_007ad894+circ_007adb50+codes_0043ba12+curves_0062ecb0+div_0060138c+state_0040fbf3")
