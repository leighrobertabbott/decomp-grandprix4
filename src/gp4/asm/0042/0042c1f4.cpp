#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `cmp left, right` on the given width.
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t mask, uint32_t sign) {
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & sign) != 0u ? 0x800u : 0u);
}
static inline uint32_t sub8(uint32_t l, uint32_t r) { return subFlags(l, r, 0xffu, 0x80u); }
static inline uint32_t sub16(uint32_t l, uint32_t r) { return subFlags(l, r, 0xffffu, 0x8000u); }
static inline uint32_t sub32(uint32_t l, uint32_t r) { return subFlags(l, r, 0xffffffffu, 0x80000000u); }

// Flags of an 8-bit logic result (test / or / and): CF = OF = AF = 0.
static inline uint32_t logic8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

static inline void setFlags(gp4::Regs& c, uint32_t flags) {
    c.eflags = (c.eflags & ~0x8d5u) | flags;
}
static inline bool carry(const gp4::Regs& c) { return (c.eflags & 1u) != 0u; }

static inline void setLow16(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffff0000u) | (value & 0xffffu);
}
static inline int16_t s16(uint32_t value) {
    return static_cast<int16_t>(static_cast<uint16_t>(value));
}
static inline uint32_t movsx16(uint32_t value) {
    return static_cast<uint32_t>(static_cast<int32_t>(s16(value)));
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
static inline void setWord(uint8_t* car, uint32_t off, uint32_t value) {
    *reinterpret_cast<uint16_t*>(car + off) = static_cast<uint16_t>(value);
}

// Each original routine is called from its own helper so the compiler cannot merge calls to
// different addresses into one call with a selected address.
template <uint32_t A>
static __declspec(noinline) void step(gp4::Regs& c) {
    gp4::call_regs(A, c);
}

// 0x0042cca0 / 0x0042ccdf: when word +0x1bc names the other car (0x66fef0 + word == ebx), bit 3 of
// +0x21d is set and bit 4 follows the sign of byte 0x604155. eax is preserved (push / pop); CF is the
// result (clc / stc) and the other flags are those of the last compare / or / and.
static void finish(gp4::Regs* r, gp4::Regs& c, uint32_t carryOut) {
    uint8_t* self = carAt(c.esi);
    const uint16_t link = word(self, 0x1bc);
    uint32_t flags = sub16(link, 0xffffu);
    if (link != 0xffffu) {
        const uint32_t target = static_cast<uint32_t>(link) + 0x0066fef0u;
        flags = sub32(c.ebx, target);
        if (c.ebx == target) {
            self[0x21d] = static_cast<uint8_t>(self[0x21d] | 8u);
            if (GP4_GLOBAL(int8_t, 0x00604155) < 0) {
                self[0x21d] = static_cast<uint8_t>(self[0x21d] | 0x10u);
            } else {
                self[0x21d] = static_cast<uint8_t>(self[0x21d] & 0xefu);
            }
            flags = logic8(self[0x21d]);
        }
    }
    c.eflags = (c.eflags & ~0x8d5u) | (flags & ~1u) | carryOut;
    *r = c;
}

// 0x0042c799 / 0x0042ca0b: steer for the gap: cx = [0x603d9c] +/- ([0x603e5e] + [0x603e3a]); unless
// the car's segment +0x1c lies in [0x868518, 0x86851c), 0x0042bb41 runs when the lane edge
// (+0x11e - [0x603d9c] / +0x11c + [0x603d9c]) is below 0x240. Then 0x0042b740 adjusts cx (stored
// at 0x6077c5), 0x004037a9 runs on (cx, [0x6077c7]) and word +0xdc = cx - [0x604402] + word +0x90;
// the flag bytes +0xcb/+0xca/+0x273 are set, 0x0042bb90 runs unless 0x6077cb is set, then
// 0x0042bdec. CF = 0.
static void steer(gp4::Regs* r, gp4::Regs& c, bool plus) {
    uint8_t* self = carAt(c.esi);
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
    c.edx = *reinterpret_cast<const uint32_t*>(self + 0x1c);
    bool inside = false;
    if (c.edx >= GP4_GLOBAL(uint32_t, 0x00868518)) {
        const uint32_t end = GP4_GLOBAL(uint32_t, 0x0086851c);
        setFlags(c, sub32(c.edx, end));
        inside = c.edx < end;
    }
    if (!inside) {
        uint16_t edge;
        if (plus) {
            edge = static_cast<uint16_t>(word(self, 0x11e) - GP4_GLOBAL(uint16_t, 0x00603d9c));
        } else {
            edge = static_cast<uint16_t>(word(self, 0x11c) + GP4_GLOBAL(uint16_t, 0x00603d9c));
        }
        setLow16(c.edx, edge);
        setFlags(c, sub16(edge, 0x240u));
        if (s16(edge) < 0x240) {
            step<0x0042bb41>(c);
        }
    }
    self = carAt(c.esi);
    setWord(self, 0x174, 8u);
    step<0x0042b740>(c);
    GP4_GLOBAL(uint16_t, 0x006077c5) = static_cast<uint16_t>(c.ecx);
    const uint32_t savedEcx = c.ecx;
    c.ecx = movsx16(c.ecx);
    c.eax = movsx16(GP4_GLOBAL(uint16_t, 0x006077c7));
    step<0x004037a9>(c);
    c.ecx = savedEcx;
    self = carAt(c.esi);
    cx = static_cast<uint16_t>(static_cast<uint16_t>(c.ecx) - GP4_GLOBAL(uint16_t, 0x00604402));
    cx = static_cast<uint16_t>(cx + word(self, 0x90));
    setLow16(c.ecx, cx);
    setWord(self, 0xdc, cx);
    self[0xcb] = static_cast<uint8_t>(self[0xcb] | 0x80u);
    self[0xcb] = static_cast<uint8_t>(self[0xcb] | 0x40u);
    self[0xca] = static_cast<uint8_t>(self[0xca] | 0x10u);
    self[0x273] = static_cast<uint8_t>(self[0x273] | 0x80u);
    self[0x273] = static_cast<uint8_t>(self[0x273] | 0x40u);
    const uint8_t committed = GP4_GLOBAL(uint8_t, 0x006077cb);
    if (committed == 0u) {
        setFlags(c, 0x44u);  // cmp byte [0x6077cb], 0 (equal)
        step<0x0042bb90>(c);
    } else {
        setFlags(c, sub8(committed, 0u));
    }
    step<0x0042bdec>(c);
    finish(r, c, 0u);
}

// The heading-error term (word +0xe - word +0x142) * [0x603c64] >> 14 (imul; shrd eax, edx, 14),
// kept in bp.
static void headingTerm(gp4::Regs& c, const uint8_t* self) {
    const int32_t error = s16(static_cast<uint32_t>(word(self, 0xe) - word(self, 0x142)));
    const int64_t product = static_cast<int64_t>(error) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603c64));
    c.eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    c.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    setLow16(c.ebp, c.eax);
}

// Common tail of the hold paths: word +0xdc = cx, 0x004037a9 on (cx, word +0x94), flag bytes and
// word +0x174 = 0x40. CF = 0.
static void holdTail(gp4::Regs* r, gp4::Regs& c, uint16_t cx) {
    uint8_t* self = carAt(c.esi);
    setLow16(c.ecx, cx);
    setWord(self, 0xdc, cx);
    c.ecx = movsx16(cx);
    c.eax = movsx16(word(self, 0x94));
    step<0x004037a9>(c);
    self = carAt(c.esi);
    self[0xcb] = static_cast<uint8_t>(self[0xcb] | 0x80u);
    self[0x273] = static_cast<uint8_t>(self[0x273] | 0x80u);
    self[0x273] = static_cast<uint8_t>(self[0x273] | 0x40u);
    setWord(self, 0x174, 0x40u);
    finish(r, c, 0u);
}

// 0x0042cac9: hold the line on the +0x11c side.
static void holdLow(gp4::Regs* r, gp4::Regs& c) {
    if (GP4_GLOBAL(int16_t, 0x00603c92) < 0) {
        finish(r, c, 1u);
        return;
    }
    const uint8_t direction = GP4_GLOBAL(uint8_t, 0x00868582);
    if ((direction & 0x80u) != 0u) {
        setFlags(c, logic8(direction));
        step<0x0042c0c4>(c);
        if (carry(c)) {
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
    const uint16_t lateral = word(self, 0x90);
    setFlags(c, sub16(cx, lateral));
    if (s16(cx) < s16(lateral)) {
        cx = lateral;
    }
    holdTail(r, c, cx);
}

// 0x0042cbb3: hold the line on the +0x11e side.
static void holdHigh(gp4::Regs* r, gp4::Regs& c) {
    if (GP4_GLOBAL(int16_t, 0x00603c92) > 0) {
        finish(r, c, 1u);
        return;
    }
    const uint8_t direction = GP4_GLOBAL(uint8_t, 0x00868582);
    if ((direction & 0x80u) == 0u) {
        setFlags(c, logic8(direction));
        step<0x0042c0c4>(c);
        if (carry(c)) {
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
    const uint16_t lateral = word(self, 0x90);
    setFlags(c, sub16(cx, lateral));
    if (s16(cx) > s16(lateral)) {
        cx = lateral;
    }
    holdTail(r, c, cx);
}

// 0x0042c598 / 0x0042c7fb: are the two cars converging fast enough? false = give up the move.
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
    setLow16(c.eax, static_cast<uint32_t>(word(self, 0x9a) - word(other, 0x9a)));
    const int32_t approach = static_cast<int32_t>(s16(c.eax)) * static_cast<int32_t>(static_cast<int16_t>(cx));
    c.edx = static_cast<uint32_t>(approach);
    c.eax = static_cast<uint32_t>(rate) << 1;
    return static_cast<int32_t>(c.edx) >= static_cast<int32_t>(c.eax);
}

// 0x0042c641 / 0x0042c8a0: room to pass: true when the gap word is small or both cars are clear of
// their limits +0xb0 by 0x200 (mirrored for the negative side). flags = those of the last compare.
static bool roomToPass(gp4::Regs& c, const uint8_t* self, const uint8_t* other, bool mirrored, uint32_t& flags) {
    setLow16(c.eax, abs16(GP4_GLOBAL(uint16_t, 0x00603e70)));
    flags = sub16(c.eax, GP4_GLOBAL(uint16_t, 0x00603dac));
    if (s16(c.eax) < GP4_GLOBAL(int16_t, 0x00603dac)) {
        return true;
    }
    setLow16(c.eax, abs16(GP4_GLOBAL(uint16_t, 0x00604404)));
    flags = sub16(c.eax, 0x200u);
    if (s16(c.eax) < 0x200) {
        return false;
    }
    uint16_t lateral = word(self, 0x90);
    if (mirrored) {
        lateral = static_cast<uint16_t>(0u - lateral);
    }
    setLow16(c.eax, static_cast<uint32_t>(lateral - 0x200u));
    flags = sub16(c.eax, word(self, 0xb0));
    if (s16(c.eax) >= s16(word(self, 0xb0))) {
        return true;
    }
    lateral = word(other, 0x90);
    if (mirrored) {
        lateral = static_cast<uint16_t>(0u - lateral);
    }
    setLow16(c.eax, static_cast<uint32_t>(lateral + 0x200u));
    flags = sub16(c.eax, word(other, 0xb0));
    return s16(c.eax) < s16(word(other, 0xb0));
}

// 0x0042c6df / 0x0042c951: the other car is inside its lane limit (+0x11c / +0x11e) and steady
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
    uint32_t flags = 0x44u;  // test byte [ebx+0xcc], 0x10 (clear)
    if ((other[0xcc] & 0x10u) != 0u) {
        const uint16_t spread = abs16(static_cast<uint32_t>(word(other, 0x90) - word(other, 0x94)));
        setLow16(c.edx, spread);
        if (s16(spread) >= 0x100) {
            return false;
        }
        flags = sub16(spread, 0x100u);
    }
    setFlags(c, flags);
    const uint32_t savedEax = c.eax;
    step<0x0042b88b>(c);
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

static void checkPass(gp4::Regs& c, bool negative) {
    if (negative) {
        step<0x0042ba87>(c);
    } else {
        step<0x0042bae4>(c);
    }
}

// 0x0042c277..0x0042c577: the engage test while both cars are slow enough inside their limits on a
// marked segment (edi +0x51 bit 7) and close (|gap| <= [0x603b52]): marks +0x179 bit 2 and either
// sets the pass flags of +0x135/+0x137/+0x140 (0x0042a0cf with [0x6223f0]) or runs the pass check
// 0x0042bae4 / 0x0042ba87 chosen by the sign of [0x604404]. false = abandon (stc).
static bool engage(gp4::Regs& c) {
    uint8_t* self = carAt(c.esi);
    const uint8_t* other = carAt(c.ebx);
    if (GP4_GLOBAL(uint8_t, 0x00603e6e) != 0u || (self[0x9c] & 2u) != 0u) {
        return true;
    }
    uint32_t speed = movsx16(word(other, 0x90));
    if ((speed & 0x80000000u) != 0u) {
        speed = 0u - speed;
    }
    c.eax = speed;
    c.edx = movsx16(word(other, 0xb0)) + 0x100u;
    if (static_cast<int32_t>(c.eax) >= static_cast<int32_t>(c.edx)) {
        return true;
    }
    speed = movsx16(word(self, 0x90));
    if ((speed & 0x80000000u) != 0u) {
        speed = 0u - speed;
    }
    c.eax = speed;
    c.edx = movsx16(word(self, 0xb0)) + 0x100u;
    if (static_cast<int32_t>(c.eax) >= static_cast<int32_t>(c.edx)) {
        return true;
    }
    const uint8_t* seg = carAt(c.edi);
    if ((seg[0x51] & 0x80u) == 0u) {
        return true;
    }
    if ((self[0x135] & 2u) != 0u && (self[0x137] & 0x40u) != 0u && (self[0xce] & 0x80u) == 0u) {
        setLow16(c.eax, GP4_GLOBAL(uint16_t, 0x00603e70));
        if (GP4_GLOBAL(int16_t, 0x00603e70) >= GP4_GLOBAL(int16_t, 0x00603dae)) {
            return false;
        }
    }
    const uint16_t gap = GP4_GLOBAL(uint16_t, 0x00603e70);
    setLow16(c.eax, gap);
    setLow16(c.edx, abs16(gap));
    if (s16(c.edx) > GP4_GLOBAL(int16_t, 0x00603b52)) {
        return true;
    }
    if ((self[0x179] & 4u) != 0u) {
        if ((self[0x135] & 2u) != 0u) {
            return true;
        }
        const bool behind = (word(self, 0x94) & 0x8000u) != 0u;
        if ((seg[0x51] & 0x40u) != 0u) {
            if (!behind) {
                return true;
            }
        } else if (behind) {
            return true;
        }
        self[0x135] = static_cast<uint8_t>(self[0x135] | 2u);
    }
    self[0x137] = 0u;
    self[0x179] = static_cast<uint8_t>(self[0x179] | 4u);

    const bool high = (seg[0x51] & 0x40u) != 0u;
    const bool negative = GP4_GLOBAL(int16_t, 0x00604404) < 0;
    const bool wide = high != negative;
    if ((self[0x122] & 4u) != 0u) {
        setFlags(c, 0u);  // test byte [esi+0x122], 4 (set)
        checkPass(c, negative);
        if (!carry(c)) {
            return true;
        }
        self = carAt(c.esi);
        self[0x122] = static_cast<uint8_t>(self[0x122] | 2u);
        self[0x122] = static_cast<uint8_t>(self[0x122] | 8u);
        self[0x122] = static_cast<uint8_t>(self[0x122] | 0x10u);
    }
    const uint16_t threshold = wide ? GP4_GLOBAL(uint16_t, 0x00603da6) : GP4_GLOBAL(uint16_t, 0x00603da8);
    const uint16_t ax = static_cast<uint16_t>(c.eax);
    if (s16(ax) >= s16(threshold)) {
        setFlags(c, sub16(ax, threshold));
        checkPass(c, negative);
        if (!carry(c)) {
            self = carAt(c.esi);
            self[0x135] = static_cast<uint8_t>(self[0x135] | 8u);
        }
        return true;
    }
    self = carAt(c.esi);
    self[0x135] = static_cast<uint8_t>(self[0x135] | 4u);
    self[0x135] = static_cast<uint8_t>(self[0x135] | 0x20u);
    if (high) {
        self[0x135] = static_cast<uint8_t>(self[0x135] | 0x10u);
    } else {
        self[0x135] = static_cast<uint8_t>(self[0x135] & 0xefu);
    }
    c.eax = wide ? GP4_GLOBAL(uint32_t, 0x00603e2c) : GP4_GLOBAL(uint32_t, 0x00603e24);
    setWord(self, 0x140, c.eax);
    self[0x137] = static_cast<uint8_t>(self[0x137] | (wide ? 0x40u : 0x80u));
    c.eax = GP4_GLOBAL(uint32_t, 0x006223f0);
    setFlags(c, logic8(self[0x137]));
    step<0x0042a0cf>(c);
    return true;
}

// FUNCTION: GP4 0x0042c1f4
// Overtaking decision between the car at esi and the car ahead at ebx (edi = the track segment):
// CF = 1 abandons the move, CF = 0 sets the target lateral word +0xdc (through 0x0042b740 /
// 0x004037a9) and the steering flags. Sibling of 0x0042cd1e with the extra engage test.
void __cdecl FUN_0042c1f4(gp4::Regs* r) {
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
        setFlags(c, sub16(ahead, mine));
    } else {
        setFlags(c, 0u);  // test byte [esi/ebx+0x9c], 2 (set)
        step<0x0040dc8a>(c);
    }
    GP4_GLOBAL(uint16_t, 0x00603e70) = static_cast<uint16_t>(c.eax);
    step<0x0042beea>(c);
    if (carry(c)) {
        finish(r, c, 1u);
        return;
    }
    step<0x0042c155>(c);
    step<0x0042b040>(c);
    step<0x0042b0d5>(c);
    step<0x0042b8bb>(c);

    if (!engage(c)) {
        finish(r, c, 1u);
        return;
    }

    self = carAt(c.esi);
    other = carAt(c.ebx);
    if (GP4_GLOBAL(int16_t, 0x00604404) >= 0) {
        if (GP4_GLOBAL(int8_t, 0x00604155) < 0) {
            uint32_t flags = 0u;
            if (!closing(c, self, other) || !roomToPass(c, self, other, false, flags)) {
                steer(r, c, false);
                return;
            }
            setFlags(c, flags);
            step<0x0042b84b>(c);
            GP4_GLOBAL(uint8_t, 0x00604155) = 0u;
            GP4_GLOBAL(uint8_t, 0x00604154) = 0u;
        }
        // 0x0042c6b1
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
        uint32_t flags = 0u;
        if (!closing(c, self, other) || !roomToPass(c, self, other, true, flags)) {
            steer(r, c, true);
            return;
        }
        setFlags(c, flags);
        step<0x0042b86b>(c);
        GP4_GLOBAL(uint8_t, 0x00604155) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x00604154) = 0xffu;
    }
    // 0x0042c916
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
GP4_IMPL(0x0042c1f4, FUN_0042c1f4, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, edi:rec[0x8943a0,0x160,0..25], ebx:rec[0x66fef0,0x41c,0..21], esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); maxinsn=2000000; globals=fuzz:i32+layout=list_0066fe3c+pair_0042e8e6+cars_0066fef0+carf_004036ca+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+rel_0042c0c4+state_0042b740+state_0042bb90+bounds_0042e330+link_0042a120+state_0042b8bb+state_004059ef+eq_0042ba87+ring_0042c1f4+cars_0042c1f4")
