#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit `sub/cmp left, right`.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x001u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x8000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit test / 32-bit or with the given result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x040u : 0u) | (result & 0x80u);
}
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x040u : 0u) | ((result & 0x80000000u) != 0u ? 0x080u : 0u);
}

static inline uint16_t word(uint32_t base, uint32_t off) {
    return GP4_FIELD(uint16_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(base)), off);
}
static inline uint8_t byte(uint32_t base, uint32_t off) {
    return GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(base)), off);
}
static inline uint32_t setLow16(uint32_t reg, uint32_t value) {
    return (reg & 0xffff0000u) | (value & 0xffffu);
}

// The car pointer of lane offset `lane` (a byte offset into the dword table 0x6041a4); lane 0 with
// the side byte 0x60414c set picks 0x6043d8 (negative side) or 0x6043dc instead.
static inline uint32_t carInLane(int32_t lane) {
    if (lane == 0 && GP4_GLOBAL(uint8_t, 0x0060414c) != 0u) {
        if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x0060414c)) < 0) {
            return GP4_GLOBAL(uint32_t, 0x006043d8);
        }
        return GP4_GLOBAL(uint32_t, 0x006043dc);
    }
    return GP4_FIELD(uint32_t, GP4_ARRAY(uint8_t, 0x006041a4), lane);
}

// FUNCTION: GP4 0x0042e97e
// Looks for the car to follow/pass from the car at esi in the lane 0x60414e (side byte 0x60414c):
// the car of that lane (0x6041a4 table) when it is ahead by less than [0x603afc] with a body gap of
// at least [0x603b2a], else the nearest qualifying car of 0x42dc22 (not [0x60427c]); the result goes
// to 0x6043e8. The cars of the word list [0x6043fc]..[0x6043f8] alongside or just behind (lap gap
// 0 or above [0x603b18]) with enough body gap ([0x603dae]/[0x603b2c]) in the same lane block the
// move (stc). For the chosen car the gap and lateral checks ([0x603db4], [0x603db6]) and the 0x42a792
// closing speed (stored at +0xd4, below [0x6039a4]: 0x603ecc = that car) decide. CF set = blocked or
// moving out of the car's own lane (+0xfe bit 6 or lane != +0xea); eax, ebx, edi and 0x603e70 are
// restored, ecx/edx and the other flags are those of the last instructions on the path.
void __cdecl FUN_0042e97e(gp4::Regs* r) {
    const uint32_t self = r->esi;
    const uint16_t savedWord = GP4_GLOBAL(uint16_t, 0x00603e70);
    gp4::Regs c = *r;
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    uint32_t edi = r->edi;
    uint32_t flags = 0;
    bool tail = false;   // continue at 0x42ec68 (else the clc exit at 0x42ec5c)

    // ax = body gap ebx - esi: words +0xe2, or 0x40dc8a when either car has +0x9c bit 1 set
    #define BODY_GAP()                                                                  \
        do {                                                                            \
            if ((byte(self, 0x9c) & 2u) == 0u && (byte(ebx, 0x9c) & 2u) == 0u) {        \
                eax = setLow16(eax, static_cast<uint32_t>(word(ebx, 0xe2) - word(self, 0xe2))); \
            } else {                                                                    \
                c.eax = eax; c.ebx = ebx; c.ecx = ecx; c.edx = edx; c.edi = edi;        \
                gp4::call_regs(0x0040dc8a, c);                                          \
                eax = c.eax;                                                            \
            }                                                                           \
        } while (0)

    GP4_GLOBAL(uint32_t, 0x00603ecc) = 0u;

    do {
        if ((byte(self, 0xe5) & 0x80u) != 0u) {
            uint16_t spread = static_cast<uint16_t>(word(self, 0xe) - word(self, 0x18));
            if ((spread & 0x8000u) != 0u) {
                spread = static_cast<uint16_t>(0u - spread);
            }
            edx = setLow16(edx, spread);
            flags = subFlags16(spread, GP4_GLOBAL(uint16_t, 0x00603d9e));
            if (static_cast<int16_t>(spread) >= GP4_GLOBAL(int16_t, 0x00603d9e)) {
                break;
            }
        }

        edi = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x0060414e)));
        edx = carInLane(static_cast<int32_t>(edi));
        GP4_GLOBAL(uint32_t, 0x006043e8) = edx;

        bool search = true;     // 0x42ea50
        bool clear = true;      // 0x42ea46
        if (edx != 0u) {
            ebx = edx;
            const uint16_t gap = static_cast<uint16_t>(word(ebx, 0x68) - word(self, 0x68));
            eax = setLow16(eax, gap);
            if (static_cast<int16_t>(gap) >= 0 && static_cast<int16_t>(gap) < GP4_GLOBAL(int16_t, 0x00603afc)) {
                clear = false;
                BODY_GAP();
                if (static_cast<int16_t>(eax) < GP4_GLOBAL(int16_t, 0x00603b2a)) {
                    tail = true;
                    break;
                }
                if (ebx == GP4_GLOBAL(uint32_t, 0x006043e0)) {
                    search = false;
                }
            }
        }
        if (clear) {
            GP4_GLOBAL(uint32_t, 0x006043e8) = 0u;
        }
        if (search) {
            c.eax = eax; c.ebx = ebx; c.ecx = ecx; c.edx = edx; c.edi = edi;
            gp4::call_regs(0x0042dc22, c);
            eax = c.eax;
            ebx = c.ebx;
            if ((c.eflags & 1u) == 0u && ebx != GP4_GLOBAL(uint32_t, 0x0060427c)) {
                const uint32_t best = GP4_GLOBAL(uint32_t, 0x006043e8);
                bool take = true;
                if (best != 0u) {
                    if (ebx == best) {
                        take = false;
                    } else {
                        edx = best;
                        eax = setLow16(eax, word(ebx, 0x68));
                        take = static_cast<int16_t>(word(ebx, 0x68)) < static_cast<int16_t>(word(best, 0x68));
                    }
                }
                if (take) {
                    GP4_GLOBAL(uint32_t, 0x006043e8) = ebx;
                }
            }
        }

        // 0x42ea89: the cars of the list
        edi = GP4_GLOBAL(uint32_t, 0x006043fc);
        while (edi != GP4_GLOBAL(uint32_t, 0x006043f8)) {
            ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x0066fef0)))
                + static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(edi))));
            edi += 2u;
            const uint16_t gap = static_cast<uint16_t>(word(ebx, 0x68) - word(self, 0x68));
            eax = setLow16(eax, gap);
            if (gap != 0u) {
                if (static_cast<int16_t>(gap) >= 0 || static_cast<int16_t>(gap) < GP4_GLOBAL(int16_t, 0x00603b18)) {
                    break;
                }
            }
            BODY_GAP();
            if ((byte(self, 0x137) & 0x40u) != 0u && (byte(self, 0xce) & 0x80u) == 0u
                && static_cast<int16_t>(eax) < GP4_GLOBAL(int16_t, 0x00603dae)) {
                break;
            }
            if (static_cast<int16_t>(eax) < GP4_GLOBAL(int16_t, 0x00603b2c)) {
                break;
            }
            const uint16_t lane = GP4_GLOBAL(uint16_t, 0x0060414e);
            edx = setLow16(edx, lane);
            if (lane != word(ebx, 0xea)) {
                continue;
            }
            if (lane != 0u || (byte(ebx, 0xfe) & 0x40u) == 0u || GP4_GLOBAL(uint8_t, 0x0060414c) == 0u) {
                tail = true;
                break;
            }
            if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x0060414c)) >= 0) {
                if ((byte(ebx, 0xfe) & 0x10u) == 0u) {
                    tail = true;
                    break;
                }
            } else if ((byte(ebx, 0xfe) & 0x10u) != 0u) {
                tail = true;
                break;
            }
        }
        if (tail) {
            break;
        }

        // 0x42eb71: the chosen car
        ebx = GP4_GLOBAL(uint32_t, 0x006043e8);
        flags = logicFlags32(ebx);
        if (ebx == 0u) {
            break;
        }
        const uint16_t gap = static_cast<uint16_t>(word(ebx, 0x68) - word(self, 0x68));
        eax = setLow16(eax, gap);
        flags = subFlags16(word(ebx, 0x68), word(self, 0x68));
        if (static_cast<int16_t>(gap) < 0) {
            break;
        }
        flags = subFlags16(gap, GP4_GLOBAL(uint16_t, 0x00603afc));
        if (static_cast<int16_t>(gap) >= GP4_GLOBAL(int16_t, 0x00603afc)) {
            break;
        }
        edi = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(word(self, 0xea))));
        eax = carInLane(static_cast<int32_t>(edi));
        if (ebx == eax) {
            BODY_GAP();
            flags = subFlags16(eax, GP4_GLOBAL(uint16_t, 0x00603db4));
            if (static_cast<int16_t>(eax) < GP4_GLOBAL(int16_t, 0x00603db4)) {
                break;
            }
            uint16_t lateral = static_cast<uint16_t>(word(self, 0x90) - word(self, 0x94) - word(ebx, 0x90) + word(ebx, 0x94));
            if ((lateral & 0x8000u) != 0u) {
                lateral = static_cast<uint16_t>(0u - lateral);
            }
            eax = setLow16(eax, lateral);
            flags = subFlags16(lateral, GP4_GLOBAL(uint16_t, 0x00603db6));
            if (static_cast<int16_t>(lateral) < GP4_GLOBAL(int16_t, 0x00603db6)) {
                break;
            }
        }

        // 0x42ec2a
        GP4_GLOBAL(uint16_t, 0x00623e08) = GP4_GLOBAL(uint16_t, 0x00623e04);
        c.eax = eax; c.ebx = ebx; c.ecx = ecx; c.edx = edx; c.edi = edi;
        gp4::call_regs(0x0042a792, c);
        eax = c.eax;
        ecx = c.ecx;
        edx = c.edx;
        GP4_FIELD(uint16_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(self)), 0xd4) = static_cast<uint16_t>(ecx);
        const uint16_t closing = word(self, 0xd4);
        edx = setLow16(edx, closing);
        flags = subFlags16(closing, GP4_GLOBAL(uint16_t, 0x006039a4));
        if (static_cast<int16_t>(closing) >= GP4_GLOBAL(int16_t, 0x006039a4)) {
            break;
        }
        GP4_GLOBAL(uint32_t, 0x00603ecc) = ebx;
        tail = true;
    } while (false);
    #undef BODY_GAP

    uint32_t carry = 0u;
    if (tail) {
        // 0x42ec68
        const uint16_t lane = GP4_GLOBAL(uint16_t, 0x0060414e);
        edx = setLow16(edx, lane);
        const uint8_t side = byte(self, 0xfe);
        flags = logicFlags8(side & 0x40u);
        if ((side & 0x40u) != 0u) {
            carry = 1u;
        } else {
            flags = subFlags16(lane, word(self, 0xea));
            carry = (lane != word(self, 0xea)) ? 1u : 0u;
        }
    }

    GP4_GLOBAL(uint16_t, 0x00603e70) = savedWord;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | carry;
}
GP4_IMPL(0x0042e97e, FUN_0042e97e, "regs(eax:u32, ebx:rec[0x66fef0,0x41c,0..21], ecx:u32, edx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (ecx, edx, cf, pf, af, zf, sf, of); cases=800; globals=layout=carbg_0066fef0+cars_0066fef0+pair_0042e8e6+flags_00430449+state_00426091+gap_0042a533+walk_0042e06a+cursor_006043fc+order_0042e97e+lane_0042e97e")
