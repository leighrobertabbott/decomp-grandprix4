#include <gp4/gp4.h>

// esi: a car record (only the fields read here are named).
struct Unknown_004442ef_Car {
    uint8_t Unknown00[0x7a];
    uint8_t Flags7A;
    uint8_t Flags7B;
    uint8_t Unknown7C[0x84 - 0x7c];
    uint8_t Unknown84;
    uint8_t Unknown85;
    uint8_t Flags86;
    uint8_t Flags87;
    uint8_t Unknown88[0x8d - 0x88];
    uint8_t Flags8D;
    uint8_t Unknown8E;
    uint8_t Flags8F;
    uint8_t Unknown90[0x9a - 0x90];
    int16_t Unknown9A;
    uint8_t Unknown9C[0x240 - 0x9c];
    uint32_t Unknown240;          // four packed per-wheel bytes
};

// edi: a 0x160-byte track-segment record (only two words are read).
struct Unknown_004442ef_Segment {
    uint8_t Unknown00[2];
    int16_t Unknown02;
    uint8_t Unknown04[0x162 - 4];
    int16_t Unknown162;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (and / or / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a 32-bit logic result.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit `cmp left, right` / `sub left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit `add left, right`.
static inline uint32_t addFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left + right) & 0xffffu;
    return (result < left ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of `shr value, 8` (oracle model: CF = last bit out, AF = 0, OF = bit 31 of the shift-7 ^ shift-8 results).
static inline uint32_t shr8Flags(uint32_t value) {
    const uint32_t result = value >> 8;
    return ((value >> 7) & 1u) | parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((((value >> 7) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit `shl ax, 5` (oracle model: CF = last bit out, AF = 0, OF = bit 15 of the shift-4 ^ shift-5 results).
static inline uint32_t shl16By5Flags(uint32_t value) {
    value &= 0xffffu;
    const uint32_t result = (value << 5) & 0xffffu;
    const uint32_t previous = (value << 4) & 0xffffu;
    return ((value >> 11) & 1u) | parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u) | (((previous ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004442ef
// Recomputes the status bits of a car after a physics step. Byte +0x86 bit 2 is cleared; when the packed per-wheel
// dword +0x240 is nonzero, byte +0x8f bits 0..3 are cleared and, with the speed word +0x9a at least 0x40, every
// wheel byte of +0x240 that is 0x10 or more sets +0x86 bit 2 and its own bit of +0x8f (bytes 0..3 -> bits 3, 2, 1, 0).
// Then, when the segment record's words +0x162 and +2 differ non-negatively and (difference * 32 + speed) is 0x4000
// or more, a random byte below [0x603854] >> 8 sets +0x86 bit 4 (0x401462 advances the generator; its three state
// words at 0x66d8be / 0x66d8c0 / 0x66d8c2 are saved and restored around everything). Finally +0x87 bit 5 is set for
// state 3 (+0x84) with +0x8d bit 4 set and none of +0x7b bit 7, +0x7a bits 6, 3, 1. eax keeps its high half and
// takes the saved word [0x66d8be]; edx is [0x603854] >> 8 when the generator ran; the flags are those of the last
// flag-setting instruction on the taken path.
void __cdecl FUN_004442ef(gp4::Regs* r) {
    Unknown_004442ef_Car* car = reinterpret_cast<Unknown_004442ef_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_004442ef_Segment* segment = reinterpret_cast<const Unknown_004442ef_Segment*>(static_cast<uintptr_t>(r->edi));

    const uint16_t saved0 = GP4_GLOBAL(uint16_t, 0x0066d8be);
    const uint16_t saved1 = GP4_GLOBAL(uint16_t, 0x0066d8c0);
    const uint16_t saved2 = GP4_GLOBAL(uint16_t, 0x0066d8c2);
    const uint32_t incomingEax = r->eax;
    uint32_t flags;

    car->Flags86 = static_cast<uint8_t>(car->Flags86 & 0xfbu);
    flags = logicFlags8(car->Flags86);

    const uint32_t packed = car->Unknown240;
    flags = logicFlags32(packed);
    if (packed != 0u) {
        car->Flags8F = static_cast<uint8_t>(car->Flags8F & 0xf7u);
        car->Flags8F = static_cast<uint8_t>(car->Flags8F & 0xfbu);
        car->Flags8F = static_cast<uint8_t>(car->Flags8F & 0xfdu);
        car->Flags8F = static_cast<uint8_t>(car->Flags8F & 0xfeu);
        flags = logicFlags8(car->Flags8F);

        flags = cmpFlags16(static_cast<uint16_t>(car->Unknown9A), 0x40u);
        if (car->Unknown9A >= 0x40) {
            static const uint8_t wheelBits[4] = {8u, 4u, 2u, 1u};
            for (uint32_t wheel = 0; wheel < 4u; ++wheel) {
                const uint32_t shifted = packed >> (8u * wheel);
                if (wheel != 0u) {
                    flags = shr8Flags(packed >> (8u * (wheel - 1u)));
                }
                const uint32_t wheelByte = shifted & 0xffu;
                flags = cmpFlags8(wheelByte, 0x10u);
                if (wheelByte >= 0x10u) {
                    car->Flags86 = static_cast<uint8_t>(car->Flags86 | 4u);
                    flags = logicFlags8(car->Flags86);
                    car->Flags8F = static_cast<uint8_t>(car->Flags8F | wheelBits[wheel]);
                    flags = logicFlags8(car->Flags8F);
                }
            }
        }
    }

    const uint16_t difference = static_cast<uint16_t>(segment->Unknown162 - segment->Unknown02);
    flags = cmpFlags16(static_cast<uint16_t>(segment->Unknown162), static_cast<uint16_t>(segment->Unknown02));
    if ((difference & 0x8000u) == 0u) {
        flags = shl16By5Flags(difference);
        const uint16_t scaled = static_cast<uint16_t>(difference << 5);
        const uint16_t total = static_cast<uint16_t>(scaled + static_cast<uint16_t>(car->Unknown9A));
        flags = addFlags16(scaled, static_cast<uint16_t>(car->Unknown9A));
        flags = cmpFlags16(total, 0x4000u);
        if (static_cast<int16_t>(total) >= 0x4000) {
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            gp4::call_regs(0x00401462, c);
            flags = c.eflags;
            const uint32_t roll = c.eax & 0xffu;
            flags = logicFlags32(roll);
            const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00603854);
            r->edx = limit >> 8;
            flags = shr8Flags(limit);
            flags = cmpFlags32(roll, limit >> 8);
            if (static_cast<int32_t>(roll) < static_cast<int32_t>(limit >> 8)) {
                car->Flags86 = static_cast<uint8_t>(car->Flags86 | 0x10u);
                flags = logicFlags8(car->Flags86);
            }
        }
    }

    flags = logicFlags8(car->Flags8D & 0x10u);
    if ((car->Flags8D & 0x10u) != 0u) {
        flags = logicFlags8(car->Flags7B & 0x80u);
        if ((car->Flags7B & 0x80u) == 0u) {
            flags = logicFlags8(car->Flags7A & 0x40u);
            if ((car->Flags7A & 0x40u) == 0u) {
                flags = logicFlags8(car->Flags7A & 8u);
                if ((car->Flags7A & 8u) == 0u) {
                    flags = logicFlags8(car->Flags7A & 2u);
                    if ((car->Flags7A & 2u) == 0u) {
                        flags = cmpFlags8(car->Unknown84, 3u);
                        if (car->Unknown84 == 3u) {
                            car->Flags87 = static_cast<uint8_t>(car->Flags87 | 0x20u);
                            flags = logicFlags8(car->Flags87);
                        }
                    }
                }
            }
        }
    }

    GP4_GLOBAL(uint16_t, 0x0066d8c2) = saved2;
    GP4_GLOBAL(uint16_t, 0x0066d8c0) = saved1;
    GP4_GLOBAL(uint16_t, 0x0066d8be) = saved0;
    r->eax = (incomingEax & 0xffff0000u) | saved0;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004442ef, FUN_004442ef, "regs(eax:u32, edx:u32, esi:ptr[0x248]:layout=car_00411109+rec_004053ff, edi:ptr[0x16c]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0+state_0041246e")
