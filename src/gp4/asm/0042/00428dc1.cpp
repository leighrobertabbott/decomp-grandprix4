#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit test result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a 16-bit logic result (or ax,ax / test ax,0xffff): CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint32_t result) {
    result &= 0xffffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit `add left, right`.
static inline uint32_t addFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left + right) & 0xffffu;
    return (result < left ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ result) & (right ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Car body fields this routine uses.
struct Unknown_00428dc1_Car {
    uint8_t Unknown00[0x71];
    uint8_t Flags71;
    uint8_t Unknown72[0x8d - 0x72];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x90 - 0x8e];
    int16_t Lateral90;
    uint8_t Unknown92[0xb0 - 0x92];
    int16_t LimitB0;
    uint8_t UnknownB2[0x174 - 0xb2];
    int16_t Raise174;
    uint8_t Unknown176[0x1a0 - 0x176];
    int32_t Fraction1A0;
};

// Track segment record (edi): flag byte +0x51 and the signed byte +0x135.
struct Unknown_00428dc1_Segment {
    uint8_t Unknown00[0x51];
    uint8_t Flags51;
    uint8_t Unknown52[0x135 - 0x52];
    int8_t Level135;
};

// FUNCTION: GP4 0x00428dc1
// Raises the word +0x174 of the car at esi to one of two thresholds (0x00603c30: the segment level
// +0x135 of edi lies inside the window [0x00627b40, 0x00627b44] and the blend fraction +0x1a0 is below
// 0x00603dec; 0x00603c2e: the car's lateral word +0x90 (sign chosen by bit 3 of +0x71 against bit 3 of
// the segment's +0x51) plus 0x00603de4 reaches +0xb0 and the lane offset 0x0042b88b is within 0x00603de6
// of +0x90). CF = 1 when it was evaluated through one of the two stores, 0 for every other exit.
// eax is saved and restored; the flags are those of the last compare.
void __cdecl FUN_00428dc1(gp4::Regs* r) {
    Unknown_00428dc1_Car* const car = reinterpret_cast<Unknown_00428dc1_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00428dc1_Segment* const segment =
        reinterpret_cast<const Unknown_00428dc1_Segment*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t flags = logicFlags8(car->Flags8D & 2u);
    uint32_t carry = 0;

    if ((car->Flags8D & 2u) == 0u) {
        const int32_t fraction = car->Fraction1A0;
        const bool segmentBit = (segment->Flags51 & 8u) != 0u;
        const bool carBit = (car->Flags71 & 8u) != 0u;

        bool blend = false;      // 0x00428e64: add the lateral word to 0x00603de4
        bool raise = false;      // 0x00428e95: the level window store
        uint16_t lateral = 0;
        if (segmentBit != carBit) {
            const int32_t limit = GP4_GLOBAL(int32_t, 0x00603de8);
            flags = cmpFlags32(static_cast<uint32_t>(fraction), static_cast<uint32_t>(limit));
            if (fraction < limit) {
                lateral = static_cast<uint16_t>(car->Lateral90);
                flags = logicFlags16(lateral);
                const bool negative = (lateral & 0x8000u) != 0u;
                if (carBit) {
                    // 0x00428ded: only a negative lateral value continues, negated
                    if (negative) {
                        lateral = static_cast<uint16_t>(0u - lateral);
                        blend = true;
                    }
                } else if (!negative) {
                    blend = true;
                }
            }
        } else {
            const int32_t limit = GP4_GLOBAL(int32_t, 0x00603dec);
            flags = cmpFlags32(static_cast<uint32_t>(fraction), static_cast<uint32_t>(limit));
            if (fraction < limit) {
                const int32_t level = segment->Level135;
                const int32_t upper = GP4_GLOBAL(int32_t, 0x00627b44);
                flags = cmpFlags32(static_cast<uint32_t>(level), static_cast<uint32_t>(upper));
                if (!(level > upper)) {
                    const int32_t lower = GP4_GLOBAL(int32_t, 0x00627b40);
                    flags = cmpFlags32(static_cast<uint32_t>(level), static_cast<uint32_t>(lower));
                    raise = level > lower;
                }
            }
        }

        bool lower = false;      // 0x00428ead: the lane store
        if (blend) {
            const uint16_t sum = static_cast<uint16_t>(lateral + GP4_GLOBAL(uint16_t, 0x00603de4));
            flags = addFlags16(lateral, GP4_GLOBAL(uint16_t, 0x00603de4));
            flags = cmpFlags16(sum, static_cast<uint16_t>(car->LimitB0));
            if (!(static_cast<int16_t>(sum) < car->LimitB0)) {
                c.eax = (c.eax & 0xffff0000u) | sum;
                c.eflags = (c.eflags & ~0x8d5u) | flags;
                gp4::call_regs(0x0042b88b, c);
                uint16_t ax = static_cast<uint16_t>(static_cast<uint16_t>(c.eax) - static_cast<uint16_t>(car->Lateral90));
                if ((ax & 0x8000u) != 0u) {
                    ax = static_cast<uint16_t>(0u - ax);
                }
                flags = cmpFlags16(ax, GP4_GLOBAL(uint16_t, 0x00603de6));
                lower = static_cast<int16_t>(ax) < GP4_GLOBAL(int16_t, 0x00603de6);
            }
        }

        if (raise || lower) {
            const int16_t store = raise ? GP4_GLOBAL(int16_t, 0x00603c30) : GP4_GLOBAL(int16_t, 0x00603c2e);
            flags = cmpFlags16(static_cast<uint16_t>(car->Raise174), static_cast<uint16_t>(store));
            if (car->Raise174 < store) {
                car->Raise174 = store;
            }
            carry = 1;
        }
    }

    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | carry;
}
GP4_IMPL(0x00428dc1, FUN_00428dc1, "regs(edi:ptr[0x13d]:bytes,esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+car_0042b17e) -> (cf,pf,af,zf,sf,of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6")
