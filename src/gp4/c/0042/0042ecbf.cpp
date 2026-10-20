#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit `cmp left, right`.
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of a 16-bit `cmp left, right`.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of a 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline uint8_t* bytesAt(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

static inline void setLow16(uint32_t& reg, uint16_t value) {
    reg = (reg & 0xffff0000u) | value;
}

// 0x0042b17e with step 0, then: is |[0x604404]| below the word [0x603ade] and bit 0 of byte +0x203 of
// ebx clear? (cx keeps the absolute value.)
static bool closeAfterLookup(gp4::Regs& s) {
    s.eax = 0;
    gp4::call_regs(0x0042b17e, s);
    uint16_t gap = GP4_GLOBAL(uint16_t, 0x00604404);
    if (static_cast<int16_t>(gap) < 0) {
        gap = static_cast<uint16_t>(0u - gap);
    }
    setLow16(s.ecx, gap);
    if (static_cast<int16_t>(gap) >= GP4_GLOBAL(int16_t, 0x00603ade)) {
        return false;
    }
    return (GP4_FIELD(uint8_t, bytesAt(s.ebx), 0x203) & 1u) == 0u;
}

enum Stage {
    kStageRear = 0,     // 0x0042ed89: bit 4 of ebx +0xcc decides on a second lookup
    kStageSpeed = 1,    // 0x0042edc7: speed of esi against [0x6013e2]
    kStageMode = 2,     // 0x0042eddb: the mode tests
    kStageClear = 3,    // 0x0042ee9b: +0xca |= 0x20, +0xcc &= ~8
    kStageSet = 4,      // 0x0042eeab: +0xca |= 0x20, +0xcc |= 8
    kStageLeave = 5     // 0x0042eee0: clc
};

// FUNCTION: GP4 0x0042ecbf
// Car esi against the car ebx on the segment record edi. While the gap word [0x603e70] is below
// [0x603b30] (unsigned) and 0x0042b17e (step 0) leaves |[0x604404]| below [0x603ade], a slower ebx
// (+0x9a) not marked by +0x7b bit 7 or +0x203 bit 0 either arms bit 6 of +0x231 with a due time
// [esi+0x250] = [0x66d814] + 5000 + 16 * (0x00401462 & 0xff), or (speed gap >= 0x1d5) clears bits 6/7.
// Otherwise the gap, the speeds (+0x9a against [0x6013e2] and 0x3ab, +0x102 / +0xc8 against
// [0x6039a2]), the segment bits (+0x51: 0x20, 0x80; +0x12: 8) and the thresholds [0x603b72/74/76/78]
// decide whether bit 3 of +0xcc is set or cleared (bit 5 of +0xca is set either way); then, with both
// bytes [0x604156]/[0x604157] = 0xff, 0x0042eee3 is asked and on CF 0x0042a0fb records (ebx, 0x6000).
// CF = 0 (clc) when nothing was decided, CF = 1 (stc) otherwise; the other flags are those of the last
// test / cmp / add / callee. Registers are not preserved: they are what the original leaves.
void __cdecl FUN_0042ecbf(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.st_xin_mask = 0;
    s.st_xout_mask = 0;

    uint32_t flags = 0;
    int stage;

    const uint16_t gap = GP4_GLOBAL(uint16_t, 0x00603e70);
    setLow16(s.eax, gap);
    if (gap >= GP4_GLOBAL(uint16_t, 0x00603b30)) {
        stage = kStageRear;
    } else if (!closeAfterLookup(s)) {
        stage = kStageClear;
    } else if ((GP4_FIELD(uint8_t, bytesAt(s.ebx), 0x7b) & 0x80u) != 0u) {
        flags = 0x80u;  // test byte [ebx+0x7b], 0x80: nonzero
        stage = kStageLeave;
    } else {
        uint8_t* car = bytesAt(s.esi);
        const uint16_t otherSpeed = GP4_FIELD(uint16_t, bytesAt(s.ebx), 0x9a);
        const uint16_t speed = GP4_FIELD(uint16_t, car, 0x9a);
        setLow16(s.edx, otherSpeed);
        if (static_cast<int16_t>(otherSpeed) >= static_cast<int16_t>(speed)) {
            stage = kStageRear;
        } else {
            const uint16_t slower = static_cast<uint16_t>(otherSpeed - speed);
            setLow16(s.edx, slower);
            if (static_cast<int16_t>(slower) >= 0x1d5) {
                GP4_FIELD(uint8_t, car, 0x231) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x231) & 0xbfu);
                GP4_FIELD(uint8_t, car, 0x231) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x231) & 0x7fu);
                stage = kStageRear;
            } else if ((GP4_FIELD(uint8_t, car, 0x231) & 0x80u) != 0u) {
                stage = kStageSpeed;
            } else if ((GP4_FIELD(uint8_t, car, 0x231) & 0x40u) != 0u) {
                flags = 0u;  // test byte [esi+0x231], 0x40: nonzero, one bit
                stage = kStageLeave;
            } else {
                GP4_FIELD(uint8_t, car, 0x231) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x231) | 0x40u);
                const uint32_t keepEax = s.eax;
                gp4::call_regs(0x00401462, s);
                const uint32_t delay = ((s.eax & 0xffu) << 4) + 0x1388u;
                s.eax = keepEax;
                const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
                GP4_FIELD(uint32_t, car, 0x250) = now;
                flags = addFlags32(GP4_FIELD(uint32_t, car, 0x250), delay);
                GP4_FIELD(uint32_t, car, 0x250) = GP4_FIELD(uint32_t, car, 0x250) + delay;
                stage = kStageLeave;
            }
        }
    }

    if (stage == kStageRear) {
        if ((GP4_FIELD(uint8_t, bytesAt(s.ebx), 0xcc) & 0x10u) == 0u) {
            stage = kStageMode;
        } else {
            stage = closeAfterLookup(s) ? kStageSpeed : kStageClear;
        }
    }

    if (stage == kStageSpeed) {
        const uint16_t speed = GP4_FIELD(uint16_t, bytesAt(s.esi), 0x9a);
        setLow16(s.edx, speed);
        stage = static_cast<int16_t>(speed) < GP4_GLOBAL(int16_t, 0x006013e2) ? kStageClear : kStageMode;
    }

    if (stage == kStageMode) {
        uint8_t* car = bytesAt(s.esi);
        uint8_t* segment = bytesAt(s.edi);
        bool leave = false;
        if (GP4_FIELD(int16_t, car, 0x9a) >= 0x3ab) {
            uint16_t level;
            if (GP4_GLOBAL(uint8_t, 0x00604156) == 0xffu && GP4_GLOBAL(uint8_t, 0x00604157) == 0xffu) {
                level = GP4_FIELD(uint16_t, car, 0x102);
            } else {
                level = GP4_FIELD(uint16_t, car, 0xc8);
            }
            setLow16(s.edx, level);
            const uint16_t minimum = GP4_GLOBAL(uint16_t, 0x006039a2);
            if (static_cast<int16_t>(level) < static_cast<int16_t>(minimum)) {
                flags = subFlags16(level, minimum);
                leave = true;
            }
        }
        if (!leave && (GP4_FIELD(uint8_t, segment, 0x51) & 0x20u) == 0u && (GP4_FIELD(uint8_t, segment, 0x12) & 8u) != 0u) {
            flags = 0u;  // test byte [edi+0x12], 8: nonzero, one bit
            leave = true;
        }
        if (leave) {
            stage = kStageLeave;
        } else {
            const uint16_t current = GP4_GLOBAL(uint16_t, 0x00603e70);
            setLow16(s.eax, current);
            bool set;
            if ((GP4_FIELD(uint8_t, car, 0xcc) & 8u) != 0u) {
                if ((GP4_FIELD(uint8_t, segment, 0x51) & 0x80u) != 0u) {
                    set = static_cast<int16_t>(current) < GP4_GLOBAL(int16_t, 0x00603b72);
                } else if ((GP4_FIELD(uint8_t, segment, 0x51) & 0x20u) != 0u) {
                    set = false;
                } else {
                    set = static_cast<int16_t>(current) < GP4_GLOBAL(int16_t, 0x00603b76);
                }
            } else {
                if ((GP4_FIELD(uint8_t, segment, 0x51) & 0x80u) != 0u) {
                    set = static_cast<int16_t>(current) < GP4_GLOBAL(int16_t, 0x00603b74);
                } else if ((GP4_FIELD(uint8_t, segment, 0x51) & 0x20u) != 0u) {
                    set = false;
                } else if (static_cast<int16_t>(current) >= GP4_GLOBAL(int16_t, 0x00603b78)) {
                    set = false;
                } else {
                    const uint16_t lateral = static_cast<uint16_t>(GP4_FIELD(uint16_t, bytesAt(s.ebx), 0xda) - GP4_FIELD(uint16_t, car, 0xda));
                    setLow16(s.edx, lateral);
                    set = static_cast<int16_t>(lateral) < 0;
                }
            }
            stage = set ? kStageSet : kStageClear;
        }
    }

    if (stage == kStageLeave) {
        r->eax = s.eax;
        r->ebx = s.ebx;
        r->ecx = s.ecx;
        r->edx = s.edx;
        r->esi = s.esi;
        r->edi = s.edi;
        r->ebp = s.ebp;
        r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u);
        return;
    }

    uint8_t* car = bytesAt(s.esi);
    GP4_FIELD(uint8_t, car, 0xca) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0xca) | 0x20u);
    if (stage == kStageSet) {
        GP4_FIELD(uint8_t, car, 0xcc) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0xcc) | 8u);
    } else {
        GP4_FIELD(uint8_t, car, 0xcc) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0xcc) & 0xf7u);
    }

    const uint8_t first = GP4_GLOBAL(uint8_t, 0x00604156);
    if (first != 0xffu) {
        flags = subFlags8(first, 0xffu);
    } else {
        const uint8_t second = GP4_GLOBAL(uint8_t, 0x00604157);
        if (second != 0xffu) {
            flags = subFlags8(second, 0xffu);
        } else {
            gp4::call_regs(0x0042eee3, s);
            flags = s.eflags;
            if ((s.eflags & 1u) != 0u) {
                s.edx = 0x6000u;
                gp4::call_regs(0x0042a0fb, s);
                flags = s.eflags;
            }
        }
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | 1u;
}
GP4_IMPL(0x0042ecbf, FUN_0042ecbf, "regs(eax:u32, ebx:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677+car_0042ecbf, ecx:u32, edx:u32, esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677+car_0042ecbf, edi:rec[0x8943a0,0x160,0..25], ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+state_0042fa52+state_0066d8c0+state_0042eee3+state_0042ecbf")
