#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `cmp left, right` / `sub left, right` on 16 bits.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | ((((left ^ right) & (left ^ result)) >> 4) & 0x800u);
}

// Flags of `cmp left, right` on 32 bits.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) >> 20) & 0x800u);
}

// Flags of an 8-bit logic result (test / or / and): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

static inline void setLow16(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffff0000u) | (value & 0xffffu);
}

static inline int16_t s16(uint32_t value) {
    return static_cast<int16_t>(static_cast<uint16_t>(value));
}

static inline uint16_t abs16(uint32_t value) {
    const uint16_t v = static_cast<uint16_t>(value);
    return (v & 0x8000u) != 0u ? static_cast<uint16_t>(0u - v) : v;
}

static inline uint8_t* carAt(uint32_t reg) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(reg));
}

static inline uint16_t word(const uint8_t* car, uint32_t off) {
    return *reinterpret_cast<const uint16_t*>(car + off);
}

static inline void callPlain(uint32_t addr, gp4::Regs& c) {
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(addr, c);
}

// 0x0042d39f / 0x0042d3de: when word +0x1bc names the other car (0x66fef0 + word == ebx), bit 3 of
// +0x21d is set and bit 4 follows the sign of byte 0x604155. eax is preserved (push / pop); CF is the
// result (clc / stc) and the other flags are those of the last compare / or / and.
static void finish(gp4::Regs* r, gp4::Regs& c, uint32_t carry) {
    uint8_t* self = carAt(c.esi);
    const uint16_t link = word(self, 0x1bc);
    uint32_t flags = subFlags16(link, 0xffffu);
    if (link != 0xffffu) {
        const uint32_t target = static_cast<uint32_t>(link) + 0x0066fef0u;
        flags = subFlags32(c.ebx, target);
        if (c.ebx == target) {
            self[0x21d] = static_cast<uint8_t>(self[0x21d] | 8u);
            if (GP4_GLOBAL(int8_t, 0x00604155) < 0) {
                self[0x21d] = static_cast<uint8_t>(self[0x21d] | 0x10u);
            } else {
                self[0x21d] = static_cast<uint8_t>(self[0x21d] & 0xefu);
            }
            flags = logicFlags8(self[0x21d]);
        }
    }
    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}

// 0x0042cfb9 / 0x0042d1e5 + 0x0042d200: steer toward the gap: cx = [0x603d9c] +/- ([0x603e5e] +
// [0x603e3a]) through 0x0042b740, stored at 0x6077c5, and the target lateral [0x603c8c] =
// cx - [0x604402] + word +0x90. CF = 0.
static void steer(gp4::Regs* r, gp4::Regs& c, bool plus) {
    setLow16(c.eax, GP4_GLOBAL(uint16_t, 0x00603e70));
    uint16_t cx = GP4_GLOBAL(uint16_t, 0x00603d9c);
    if (plus) {
        cx = static_cast<uint16_t>(cx + GP4_GLOBAL(uint16_t, 0x00603e5e));
        cx = static_cast<uint16_t>(cx + GP4_GLOBAL(uint16_t, 0x00603e3a));
    } else {
        cx = static_cast<uint16_t>(cx - GP4_GLOBAL(uint16_t, 0x00603e5e));
        cx = static_cast<uint16_t>(cx - GP4_GLOBAL(uint16_t, 0x00603e3a));
    }
    setLow16(c.ecx, cx);
    callPlain(0x0042b740, c);
    GP4_GLOBAL(uint16_t, 0x006077c5) = static_cast<uint16_t>(c.ecx);
    const uint8_t* self = carAt(c.esi);
    cx = static_cast<uint16_t>(static_cast<uint16_t>(c.ecx) - GP4_GLOBAL(uint16_t, 0x00604402));
    cx = static_cast<uint16_t>(cx + word(self, 0x90));
    setLow16(c.ecx, cx);
    GP4_GLOBAL(uint16_t, 0x00603c8c) = cx;
    finish(r, c, 0u);
}

// The heading-error term (word +0xe - word +0x142) * [0x603c64] >> 14 (imul; shrd eax, edx, 14).
static void headingTerm(gp4::Regs& c, const uint8_t* self) {
    const int32_t error = s16(static_cast<uint32_t>(word(self, 0xe) - word(self, 0x142)));
    const int64_t product = static_cast<int64_t>(error) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603c64));
    c.eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    c.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    setLow16(c.ebp, c.eax);
}

// 0x0042d226: hold the line on the +0x11c side.
static void holdLow(gp4::Regs* r, gp4::Regs& c) {
    if (GP4_GLOBAL(int16_t, 0x00603c92) < 0) {
        finish(r, c, 1u);
        return;
    }
    if (GP4_GLOBAL(int8_t, 0x00868582) < 0) {
        callPlain(0x0042c0c4, c);
        if ((c.eflags & 1u) != 0u) {
            finish(r, c, 1u);
            return;
        }
    }
    const uint8_t* self = carAt(c.esi);
    if (word(self, 0xea) == 0u) {
        finish(r, c, 1u);
        return;
    }
    if ((self[0x162] & 8u) != 0u && GP4_GLOBAL(int8_t, 0x00868582) < 0) {
        finish(r, c, 1u);
        return;
    }
    headingTerm(c, self);
    uint16_t ax = static_cast<uint16_t>(c.eax);
    ax = static_cast<uint16_t>(ax + word(self, 0x90));
    ax = static_cast<uint16_t>(ax + word(self, 0x11c));
    setLow16(c.eax, ax);
    if ((ax & 0x8000u) == 0u) {
        finish(r, c, 1u);
        return;
    }
    uint16_t bp = static_cast<uint16_t>(0u - static_cast<uint16_t>(c.ebp));
    if (static_cast<int16_t>(bp) < 0) {
        bp = 0u;
    }
    if (static_cast<int16_t>(bp) > GP4_GLOBAL(int16_t, 0x00603b20)) {
        bp = GP4_GLOBAL(uint16_t, 0x00603b20);
    }
    setLow16(c.ebp, bp);
    uint16_t cx = static_cast<uint16_t>(static_cast<uint16_t>(0u - word(self, 0x11c)) + bp);
    if (static_cast<int16_t>(cx) < s16(word(self, 0x90))) {
        cx = word(self, 0x90);
    }
    setLow16(c.ecx, cx);
    GP4_GLOBAL(uint16_t, 0x00603c8c) = cx;
    finish(r, c, 0u);
}

// 0x0042d2e3: hold the line on the +0x11e side.
static void holdHigh(gp4::Regs* r, gp4::Regs& c) {
    const int16_t side = GP4_GLOBAL(int16_t, 0x00603c92);
    if (side > 0) {
        finish(r, c, 1u);
        return;
    }
    if (GP4_GLOBAL(int8_t, 0x00868582) >= 0) {
        callPlain(0x0042c0c4, c);
        if ((c.eflags & 1u) != 0u) {
            finish(r, c, 1u);
            return;
        }
    }
    const uint8_t* self = carAt(c.esi);
    if (word(self, 0xea) == 0u) {
        finish(r, c, 1u);
        return;
    }
    if ((self[0x162] & 8u) != 0u && GP4_GLOBAL(int8_t, 0x00868582) >= 0) {
        finish(r, c, 1u);
        return;
    }
    headingTerm(c, self);
    uint16_t ax = static_cast<uint16_t>(c.eax);
    ax = static_cast<uint16_t>(ax + word(self, 0x90));
    ax = static_cast<uint16_t>(0u - ax);
    ax = static_cast<uint16_t>(ax + word(self, 0x11e));
    setLow16(c.eax, ax);
    if ((ax & 0x8000u) == 0u) {
        finish(r, c, 1u);
        return;
    }
    uint16_t bp = static_cast<uint16_t>(c.ebp);
    if (static_cast<int16_t>(bp) < 0) {
        bp = 0u;
    }
    if (static_cast<int16_t>(bp) > GP4_GLOBAL(int16_t, 0x00603b20)) {
        bp = GP4_GLOBAL(uint16_t, 0x00603b20);
    }
    setLow16(c.ebp, bp);
    uint16_t cx = static_cast<uint16_t>(word(self, 0x11e) - bp);
    if (static_cast<int16_t>(cx) > s16(word(self, 0x90))) {
        cx = word(self, 0x90);
    }
    setLow16(c.ecx, cx);
    GP4_GLOBAL(uint16_t, 0x00603c8c) = cx;
    finish(r, c, 0u);
}

// 0x0042cdbd / 0x0042cfe6: are the two cars converging fast enough? false = give up the move.
// Lateral difference (+0x90) against the sign byte 0x604153, then (unless the gap word
// [0x603e70] is below [0x603b52]) the closing product (+0x9a difference * signed [0x604404] +
// [0x603ae0]) against twice gap * rate ([0x6039ce] / [0x6039d0] by the sign of +0x184).
static bool closing(gp4::Regs& c, const uint8_t* self, const uint8_t* other) {
    c.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int8_t, 0x00604153)));
    setLow16(c.edx, static_cast<uint32_t>(word(self, 0x90) - word(other, 0x90)));
    setLow16(c.eax, c.eax ^ c.edx);
    if ((c.eax & 0x8000u) == 0u) {
        return false;
    }
    uint16_t cx = GP4_GLOBAL(uint16_t, 0x00604404);
    if (GP4_GLOBAL(int8_t, 0x00604153) >= 0) {
        cx = static_cast<uint16_t>(0u - cx);
    }
    setLow16(c.ecx, cx);
    setLow16(c.eax, static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00603e70) - GP4_GLOBAL(uint16_t, 0x00603b52)));
    if ((c.eax & 0x8000u) != 0u) {
        return true;
    }
    cx = static_cast<uint16_t>(cx + GP4_GLOBAL(uint16_t, 0x00603ae0));
    setLow16(c.ecx, cx);
    if ((cx & 0x8000u) != 0u) {
        return false;
    }
    setLow16(c.edx, GP4_GLOBAL(uint16_t, 0x006039ce));
    if ((self[0x179] & 0x80u) != 0u) {
        c.edx = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int8_t, 0x00604153)));
        setLow16(c.edx, c.edx ^ word(self, 0x184));
        if ((c.edx & 0x8000u) != 0u) {
            setLow16(c.edx, GP4_GLOBAL(uint16_t, 0x006039ce));
        } else {
            setLow16(c.edx, GP4_GLOBAL(uint16_t, 0x006039d0));
        }
    }
    const int32_t rate = static_cast<int32_t>(s16(c.eax)) * static_cast<int32_t>(s16(c.edx));
    setLow16(c.eax, static_cast<uint32_t>(rate));
    setLow16(c.eax, static_cast<uint32_t>(word(self, 0x9a) - word(other, 0x9a)));
    const int32_t approach = static_cast<int32_t>(s16(c.eax)) * static_cast<int32_t>(static_cast<int16_t>(cx));
    c.edx = static_cast<uint32_t>(approach);
    c.eax = static_cast<uint32_t>(rate) << 1;
    return static_cast<int32_t>(c.edx) >= static_cast<int32_t>(c.eax);
}

// 0x0042ce66 / 0x0042d087: room to pass: true when the gap word is small or both cars are clear of
// their limits +0xb0 by 0x200 (mirrored for the negative side).
static bool roomToPass(gp4::Regs& c, const uint8_t* self, const uint8_t* other, bool mirrored) {
    setLow16(c.eax, abs16(GP4_GLOBAL(uint16_t, 0x00603e70)));
    if (s16(c.eax) < GP4_GLOBAL(int16_t, 0x00603dac)) {
        return true;
    }
    setLow16(c.eax, abs16(GP4_GLOBAL(uint16_t, 0x00604404)));
    if (s16(c.eax) < 0x200) {
        return false;
    }
    uint16_t lateral = word(self, 0x90);
    if (mirrored) {
        lateral = static_cast<uint16_t>(0u - lateral);
    }
    setLow16(c.eax, static_cast<uint32_t>(lateral - 0x200u));
    if (s16(c.eax) >= s16(word(self, 0xb0))) {
        return true;
    }
    lateral = word(other, 0x90);
    if (mirrored) {
        lateral = static_cast<uint16_t>(0u - lateral);
    }
    setLow16(c.eax, static_cast<uint32_t>(lateral + 0x200u));
    return s16(c.eax) < s16(word(other, 0xb0));
}

// 0x0042cf42 / 0x0042d176: the other car is inside its lane limit (+0x11c / +0x11e) and steady
// (+0xcc bit 4: |+0x90 - +0x94| < 0x100), and the lane edge of 0x0042b88b plus the table word
// 0x6041a8[+0xea] passes word +0x90 of this car. true = commit (set 0x6077cb).
static bool laneOpen(gp4::Regs& c, bool high) {
    const uint8_t* self = carAt(c.esi);
    const uint8_t* other = carAt(c.ebx);
    if ((self[0x8d] & 2u) != 0u || (self[0x1ce] & 1u) != 0u || (self[0x9c] & 2u) != 0u) {
        return false;
    }
    if (word(self, 0xea) == 0u || GP4_GLOBAL(uint8_t, 0x00603e6e) != 0u) {
        return false;
    }
    setLow16(c.edx, abs16(word(other, 0x90)));
    setLow16(c.edx, static_cast<uint32_t>(c.edx - word(other, high ? 0x11e : 0x11c)));
    if ((c.edx & 0x8000u) == 0u) {
        return false;
    }
    if ((other[0xcc] & 0x10u) != 0u) {
        setLow16(c.edx, abs16(static_cast<uint32_t>(word(other, 0x90) - word(other, 0x94))));
        if (s16(c.edx) >= 0x100) {
            return false;
        }
    }
    const uint32_t savedEax = c.eax;
    callPlain(0x0042b88b, c);
    c.edx = c.eax;
    c.eax = savedEax;
    const uint8_t* table = GP4_ARRAY(uint8_t, 0x006041a8);
    const uint8_t* me = carAt(c.esi);
    const int32_t state = s16(word(me, 0xea));
    setLow16(c.edx, static_cast<uint32_t>(c.edx + *reinterpret_cast<const uint16_t*>(table + state)));
    if (high) {
        return s16(c.edx) > s16(word(me, 0x90));
    }
    return s16(c.edx) < s16(word(me, 0x90));
}

// FUNCTION: GP4 0x0042cd1e
// Overtaking decision between the car at esi and the car ahead at ebx: CF = 1 abandons the move,
// CF = 0 leaves the target lateral position in [0x603c8c].
void __cdecl FUN_0042cd1e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    GP4_GLOBAL(uint8_t, 0x006077cb) = 0u;
    GP4_GLOBAL(uint8_t, 0x0060440c) = 0u;
    GP4_GLOBAL(uint16_t, 0x00603c92) = 0u;
    const uint8_t* self = carAt(c.esi);
    const uint8_t* other = carAt(c.ebx);
    if ((self[0x9c] & 2u) != 0u) {
        uint16_t side = 0x8000u;
        if (GP4_GLOBAL(int8_t, 0x00868582) < 0) {
            side = 0x4000u;
        }
        setLow16(c.eax, side);
        GP4_GLOBAL(uint16_t, 0x00603c92) = side;
    }

    if ((self[0x9c] & 2u) == 0u && (other[0x9c] & 2u) == 0u) {
        const uint16_t ahead = word(other, 0xe2);
        const uint16_t mine = word(self, 0xe2);
        setLow16(c.eax, static_cast<uint32_t>(ahead - mine));
        c.eflags = (c.eflags & ~0x8d5u) | subFlags16(ahead, mine);
    } else {
        c.eflags = (c.eflags & ~0x8d5u) | logicFlags8(2u);
        callPlain(0x0040dc8a, c);
    }
    GP4_GLOBAL(uint16_t, 0x00603e70) = static_cast<uint16_t>(c.eax);
    callPlain(0x0042beea, c);
    if ((c.eflags & 1u) != 0u) {
        finish(r, c, 1u);
        return;
    }
    callPlain(0x0042b040, c);
    callPlain(0x0042c155, c);
    callPlain(0x0042b0d5, c);
    callPlain(0x0042b8bb, c);
    self = carAt(c.esi);
    other = carAt(c.ebx);

    if (GP4_GLOBAL(int16_t, 0x00604404) >= 0) {
        if (GP4_GLOBAL(int8_t, 0x00604155) < 0) {
            if (!closing(c, self, other) || !roomToPass(c, self, other, false)) {
                steer(r, c, false);
                return;
            }
            GP4_GLOBAL(uint8_t, 0x00604155) = 0u;
            GP4_GLOBAL(uint8_t, 0x00604154) = 0u;
        }
        // 0x0042ced1
        setLow16(c.eax, GP4_GLOBAL(uint16_t, 0x00603e70));
        const uint16_t cx = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00604404) - GP4_GLOBAL(uint16_t, 0x00603e5e));
        setLow16(c.ecx, cx);
        if (static_cast<int16_t>(cx) < GP4_GLOBAL(int16_t, 0x00603e38)) {
            steer(r, c, true);
            return;
        }
        if (static_cast<int16_t>(cx) <= GP4_GLOBAL(int16_t, 0x00603e3c) || !laneOpen(c, false)) {
            holdHigh(r, c);
            return;
        }
        GP4_GLOBAL(uint8_t, 0x006077cb) = 0xffu;
        steer(r, c, true);
        return;
    }

    if (GP4_GLOBAL(int8_t, 0x00604155) >= 0) {
        if (!closing(c, self, other) || !roomToPass(c, self, other, true)) {
            steer(r, c, true);
            return;
        }
        GP4_GLOBAL(uint8_t, 0x00604155) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x00604154) = 0xffu;
    }
    // 0x0042d0f8
    setLow16(c.eax, GP4_GLOBAL(uint16_t, 0x00603e70));
    const uint16_t cx = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00604404) + GP4_GLOBAL(uint16_t, 0x00603e5e));
    setLow16(c.ecx, cx);
    setLow16(c.edx, abs16(cx));
    if (s16(c.edx) < GP4_GLOBAL(int16_t, 0x00603e38)) {
        steer(r, c, false);
        return;
    }
    if (s16(c.edx) <= GP4_GLOBAL(int16_t, 0x00603e3c) || !laneOpen(c, true)) {
        holdLow(r, c);
        return;
    }
    GP4_GLOBAL(uint8_t, 0x006077cb) = 0xffu;
    steer(r, c, false);
}
GP4_IMPL(0x0042cd1e, FUN_0042cd1e, "regs(eax:u32, ecx:u32, edx:u32, edi:u32, ebp:u32, ebx:rec[0x66fef0,0x41c,0..21], esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); maxinsn=400000; globals=fuzz:i32+layout=list_0066fe3c+pair_0042e8e6+cars_0066fef0+carf_004036ca+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+rel_0042c0c4+state_0042b740+state_0042bb90+bounds_0042e330+link_0042a120+state_0042b8bb")
