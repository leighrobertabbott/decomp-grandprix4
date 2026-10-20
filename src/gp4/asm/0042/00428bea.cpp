#include <gp4/gp4.h>

// esi: a car record (only the fields used here are named).
struct Unknown_00428bea_Car {
    uint8_t Unknown00[0x71];
    uint8_t Flags71;
    uint8_t Unknown72[0x1b];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x71];
    uint8_t FlagsFF;
    uint8_t Unknown100[0xa];
    uint16_t Unknown10A;
    uint8_t Unknown10C[0x55];
    uint8_t Flags161;
    uint8_t Unknown162;
    uint8_t Flags163;
    uint8_t Unknown164[0x18];
    uint32_t Unknown17C;     // due time against the counter 0x66d814
    uint8_t Unknown180[0xc8];
    int16_t Unknown248;
    uint8_t Unknown24A[0x4b];
    uint8_t Unknown295;
};

// edi: the record whose bytes +0x10, +0x12 and +0x51 steer the choice.
struct Unknown_00428bea_Other {
    uint8_t Unknown00[0x10];
    uint8_t Unknown10;
    uint8_t Unknown11;
    uint8_t Flags12;
    uint8_t Unknown13[0x3e];
    uint8_t Flags51;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((((left ^ result) & (right ^ result)) >> 31) << 11);
}

// imul; shrd eax, edx, 14
static inline uint32_t mulQ14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * b;
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x00428bea
// Unless byte +0x161 has bit 7 or bit 3, or the due time +0x17c is set and not yet reached by the
// counter 0x66d814, draws a random byte (0x00401462) into the signed word +0x248 =
// +-(0x4000 - 64 * byte) (sign from the byte's bit 0, then forced by edi+0x10 bit 7 and car
// +0x71 bit 3) and sets the next due time to 0x66d814 + 4 * random byte + 0xfa. Then stores
// 0x00403a1e(edi) at +0x295, records the sign of +0x248 in bit 1 of +0xff and sets word
// +0x10a = 0x4000 - ((0x4000 - |+0x248|) * gain >> 14), with gain 0x603cc8 / 0x603ccc / 0x603cc4
// (bit 1 of +0x163 set) or, in the normal case, 0x603cc0 times the Q14 factor
// clamp(((edi+0x10 & 0x7f) + 1) * 128 * [0x622374] >> 14, [0x622378], 0x4000) stored at 0x62237c.
// eax, edx (high product dword) and ebp change; the flags are those of the final add.
void __cdecl FUN_00428bea(gp4::Regs* r) {
    Unknown_00428bea_Car* car = reinterpret_cast<Unknown_00428bea_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00428bea_Other* other =
        reinterpret_cast<const Unknown_00428bea_Other*>(static_cast<uintptr_t>(r->edi));
    uint32_t ebp = r->ebp;

    bool redraw = false;
    if ((car->Flags161 & 0x80u) == 0u && (car->Flags161 & 8u) == 0u) {
        const uint32_t due = car->Unknown17C;
        redraw = due == 0u || static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d814) - due) >= 0;
    }
    if (redraw) {
        gp4::Regs s = *r;
        gp4::call_regs(0x00401462, s);
        const uint32_t roll = s.eax & 0xffu;
        uint32_t value = 0x4000u - (roll << 6);
        ebp = roll >> 1;
        if ((roll & 1u) != 0u) {
            value = 0u - value;
        }
        if ((other->Unknown10 & 0x80u) != 0u) {
            if ((car->Flags71 & 8u) == 0u) {
                if (static_cast<int32_t>(value) < 0) {
                    value = 0u - value;
                }
                value = 0u - value;
            }
        } else if ((car->Flags71 & 8u) != 0u && static_cast<int32_t>(value) < 0) {
            value = 0u - value;
        }
        car->Unknown248 = static_cast<int16_t>(value);
        s.eax = value;
        s.ebp = ebp;
        gp4::call_regs(0x00401462, s);
        car->Unknown17C = GP4_GLOBAL(uint32_t, 0x0066d814) + ((s.eax & 0xffu) << 2) + 0xfau;
    }

    gp4::Regs t = *r;
    t.eax = r->edi;
    gp4::call_regs(0x00403a1e, t);
    car->Unknown295 = static_cast<uint8_t>(t.eax);

    car->FlagsFF &= 0xfdu;
    int32_t magnitude = car->Unknown248;
    if (magnitude < 0) {
        magnitude = -magnitude;
        car->FlagsFF |= 2u;
    }
    const int32_t rest = 0x4000 - magnitude;

    int64_t product;
    if ((car->Flags8D & 2u) != 0u) {
        car->Flags163 |= 2u;
        product = static_cast<int64_t>(rest) * GP4_GLOBAL(int32_t, 0x00603cc8);
    } else if ((car->Flags161 & 0x80u) != 0u) {
        car->Flags163 |= 2u;
        product = static_cast<int64_t>(rest) * GP4_GLOBAL(int32_t, 0x00603ccc);
    } else if ((other->Flags12 & 4u) != 0u || (other->Flags51 & 0x20u) != 0u || (car->Flags163 & 1u) != 0u) {
        car->Flags163 |= 2u;
        product = static_cast<int64_t>(rest) * GP4_GLOBAL(int32_t, 0x00603cc4);
    } else {
        car->Flags163 &= 0xfdu;
        const uint32_t scaled = mulQ14(rest, GP4_GLOBAL(int32_t, 0x00603cc0));
        int32_t factor = static_cast<int32_t>(mulQ14(static_cast<int32_t>(((other->Unknown10 & 0x7fu) + 1u) << 7),
                                                     GP4_GLOBAL(int32_t, 0x00622374)));
        if (factor < GP4_GLOBAL(int32_t, 0x00622378)) {
            factor = GP4_GLOBAL(int32_t, 0x00622378);
        }
        if (factor > 0x4000) {
            factor = 0x4000;
        }
        GP4_GLOBAL(int32_t, 0x0062237c) = factor;
        product = static_cast<int64_t>(factor) * static_cast<int32_t>(scaled);
    }

    const uint32_t low = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t result = 0x4000u - low;
    car->Unknown10A = static_cast<uint16_t>(result);

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(0u - low, 0x4000u);
}
GP4_IMPL(0x00428bea, FUN_00428bea, "regs(esi:ptr[0x29d]:layout=car_00428bea, edi:rec[0x8943a0,0x160,0..25]) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); cases=400; globals=layout=rng_0066d8c0+state_0043c4d9+gain_00428bea+seg_00428bea")
