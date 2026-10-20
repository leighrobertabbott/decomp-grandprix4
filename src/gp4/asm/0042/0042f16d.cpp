#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a logic op (test / and / or) on an 8-bit result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a logic op on a 32-bit result.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// Flags of the 16-bit "cmp lhs, rhs" / "sub lhs, rhs".
static inline uint32_t subFlags16(uint32_t lhs, uint32_t rhs) {
    lhs &= 0xffffu;
    rhs &= 0xffffu;
    const uint32_t result = (lhs - rhs) & 0xffffu;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 8) & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit "cmp lhs, rhs".
static inline uint32_t subFlags32(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = lhs - rhs;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit "add lhs, rhs".
static inline uint32_t addFlags32(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = lhs + rhs;
    return (result < lhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline void callRoutine(uint32_t addr, gp4::Regs& s, uint32_t& flags) {
    s.eflags = (s.eflags & ~0x8d5u) | flags;
    gp4::call_regs(addr, s);
    flags = s.eflags & 0x8d5u;
}

// FUNCTION: GP4 0x0042f16d
// esi = car, edi = its track segment. Maintains the state bits 2/4/8/0x10 of byte +0x122 against
// the linked car [esi+0x124] (ebx): the +0x68 gap and the gap word ax (0x40dc8a, or the +0xe2
// difference; also stored into 0x603e70) are compared with the windows 0x603b10/0x603b3c/0x603b34;
// inside them the bits advance through the due time +0xf0 (race clock 0x66d814), the 0x42d665 and
// 0x42a278 checks, and bit 8 of +0xca; outside them the bits are cleared and 0x401462 may re-arm
// the due time (clock + 5000, bit 4). Registers are left as the original's path leaves them; the
// flags are those of the last flag writer.
void __cdecl FUN_0042f16d(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const seg = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    uint32_t flags;

    uint32_t state = GP4_FIELD(uint8_t, car, 0x122);
    flags = logicFlags8(state & 2u);
    if ((state & 2u) == 0u) {
        flags = logicFlags8(state & 4u);
        if ((state & 4u) != 0u) {
            const uint32_t due = GP4_FIELD(uint32_t, car, 0xf0);
            flags = logicFlags32(due);
            if (due != 0u) {
                const uint32_t left = GP4_GLOBAL(uint32_t, 0x0066d814) + (0u - due);
                flags = addFlags32(0u - due, GP4_GLOBAL(uint32_t, 0x0066d814));
                if ((left & 0x80000000u) == 0u) {
                    const uint32_t value = GP4_FIELD(uint8_t, car, 0x122) & 0xfbu;
                    GP4_FIELD(uint8_t, car, 0x122) = static_cast<uint8_t>(value);
                    flags = logicFlags8(value);
                }
            }
        }
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    bool reset = true;   // 0x42f372
    const uint32_t mode = GP4_FIELD(uint16_t, car, 0xea);
    flags = subFlags16(mode, 0u);
    if (mode == 0u) {
        uint8_t* const other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, car, 0x124)));
        s.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(other));
        const uint32_t mine = GP4_FIELD(uint16_t, car, 0x68);
        const uint32_t theirs = GP4_FIELD(uint16_t, other, 0x68);
        const uint32_t gap = (mine - theirs) & 0xffffu;
        s.edx = (s.edx & 0xffff0000u) | gap;
        const uint32_t window = GP4_GLOBAL(uint16_t, 0x00603b10);
        flags = subFlags16(gap, window);
        if (static_cast<int16_t>(gap) < static_cast<int16_t>(window)) {
            const uint32_t ownBits = GP4_FIELD(uint8_t, car, 0x9c);
            flags = logicFlags8(ownBits & 2u);
            bool viaHelper = (ownBits & 2u) != 0u;
            if (!viaHelper) {
                const uint32_t otherBits = GP4_FIELD(uint8_t, other, 0x9c);
                flags = logicFlags8(otherBits & 2u);
                viaHelper = (otherBits & 2u) != 0u;
            }
            if (viaHelper) {
                callRoutine(0x0040dc8a, s, flags);
            } else {
                const uint32_t a = GP4_FIELD(uint16_t, other, 0xe2);
                const uint32_t b = GP4_FIELD(uint16_t, car, 0xe2);
                flags = subFlags16(a, b);
                s.eax = (s.eax & 0xffff0000u) | ((a - b) & 0xffffu);
            }
            const uint32_t ax = s.eax & 0xffffu;
            GP4_GLOBAL(uint16_t, 0x00603e70) = static_cast<uint16_t>(ax);

            bool inside = true;
            const uint32_t low = GP4_GLOBAL(uint16_t, 0x00603b3c);
            flags = subFlags16(ax, low);
            if (static_cast<int16_t>(ax) < static_cast<int16_t>(low)) {
                const uint32_t marked = GP4_GLOBAL(uint32_t, 0x006043e4);
                flags = subFlags32(s.ebx, marked);
                inside = (s.ebx == marked);
            }
            if (inside) {
                const uint32_t high = GP4_GLOBAL(uint16_t, 0x00603b34);
                flags = subFlags16(ax, high);
                inside = static_cast<int16_t>(ax) < static_cast<int16_t>(high);
            }
            if (inside) {
                const uint32_t lane = GP4_FIELD(uint8_t, car, 0x71);
                flags = logicFlags8(lane & 0x80u);
                if ((lane & 0x80u) != 0u) {
                    const uint32_t where = GP4_FIELD(uint32_t, car, 0x1c);
                    const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);
                    flags = subFlags32(where, pitStart);
                    if (where < pitStart) {
                        const uint32_t ahead = where + 0x6e00u;
                        const uint32_t pitEntry = GP4_GLOBAL(uint32_t, 0x00866f00);
                        flags = subFlags32(ahead, pitEntry);
                        inside = ahead < pitEntry;
                    }
                }
            }

            if (inside) {
                reset = false;
                state = GP4_FIELD(uint8_t, car, 0x122);
                flags = logicFlags8(state & 0x10u);
                if ((state & 0x10u) != 0u) {
                    const uint32_t close = GP4_GLOBAL(uint16_t, 0x00603b38);
                    flags = subFlags16(ax, close);
                    if (static_cast<int16_t>(ax) < static_cast<int16_t>(close)) {
                        reset = true;
                    } else {
                        const uint32_t edge = seg[0x51];
                        flags = logicFlags8(edge & 0x80u);
                        if ((edge & 0x80u) != 0u) {
                            const uint32_t speed = GP4_FIELD(uint16_t, car, 0x9a);
                            s.eax = (s.eax & 0xffff0000u) | speed;
                            const uint32_t otherSpeed = GP4_FIELD(uint16_t, other, 0x9a);
                            flags = subFlags16(speed, otherSpeed);
                            if (static_cast<int16_t>(speed) >= static_cast<int16_t>(otherSpeed)) {
                                const uint32_t value = GP4_FIELD(uint8_t, car, 0xca) | 8u;
                                GP4_FIELD(uint8_t, car, 0xca) = static_cast<uint8_t>(value);
                                flags = logicFlags8(value);
                            }
                        }
                    }
                } else {
                    bool armed = false;    // reached 0x42f2f2 (bit 8)
                    flags = logicFlags8(state & 8u);
                    if ((state & 8u) != 0u) {
                        armed = true;
                    } else {
                        bool pending = false;  // reached 0x42f2a6 (bit 4)
                        flags = logicFlags8(state & 4u);
                        if ((state & 4u) != 0u) {
                            pending = true;
                        } else {
                            const uint32_t due = GP4_FIELD(uint32_t, car, 0xf0);
                            flags = logicFlags32(due);
                            if (due != 0u) {
                                const uint32_t left = GP4_GLOBAL(uint32_t, 0x0066d814) + (0u - due);
                                flags = addFlags32(0u - due, GP4_GLOBAL(uint32_t, 0x0066d814));
                                if ((left & 0x80000000u) == 0u) {
                                    const uint32_t value = GP4_FIELD(uint8_t, car, 0x122) | 4u;
                                    GP4_FIELD(uint8_t, car, 0x122) = static_cast<uint8_t>(value);
                                    flags = logicFlags8(value);
                                    pending = true;
                                }
                            }
                        }
                        if (pending) {
                            uint32_t slope = GP4_FIELD(uint16_t, seg, 0x18);
                            flags = parityFlag(slope) | (slope == 0u ? 0x40u : 0u) | ((slope >> 8) & 0x80u);
                            if ((slope & 0x8000u) != 0u) {
                                flags = subFlags16(0u, slope);
                                slope = (0u - slope) & 0xffffu;
                            }
                            s.edx = (s.edx & 0xffff0000u) | slope;
                            const uint32_t limit = GP4_GLOBAL(uint16_t, 0x00603ec8);
                            flags = subFlags16(slope, limit);
                            if (static_cast<int16_t>(slope) < static_cast<int16_t>(limit)) {
                                const uint32_t near = GP4_GLOBAL(uint16_t, 0x00603b36);
                                flags = subFlags16(ax, near);
                                if (static_cast<int16_t>(ax) >= static_cast<int16_t>(near)) {
                                    const uint32_t lock = GP4_FIELD(uint8_t, car, 0x179);
                                    flags = logicFlags8(lock & 0x80u);
                                    if ((lock & 0x80u) == 0u) {
                                        s.ecx = 0x10u;
                                        callRoutine(0x0042d665, s, flags);
                                        if ((flags & 1u) == 0u) {
                                            const uint32_t value = GP4_FIELD(uint8_t, car, 0x122) | 8u;
                                            GP4_FIELD(uint8_t, car, 0x122) = static_cast<uint8_t>(value);
                                            flags = logicFlags8(value);
                                            armed = true;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (armed) {
                        bool check = true;
                        const uint32_t follower = GP4_FIELD(uint16_t, other, 0x1bc);
                        flags = subFlags16(follower, 0xffffu);
                        if (follower != 0xffffu) {
                            s.edx = follower + 0x0066fef0u;
                            flags = subFlags32(r->esi, s.edx);
                            if (r->esi == s.edx) check = false;
                        }
                        bool run = true;
                        if (check) {
                            const uint32_t diff = (GP4_FIELD(uint16_t, other, 0x9a) - GP4_FIELD(uint16_t, car, 0x9a)) & 0xffffu;
                            const uint32_t closing = static_cast<uint16_t>(static_cast<int16_t>(diff) >> 4);
                            s.edx = (s.edx & 0xffff0000u) | closing;
                            const uint32_t negated = (0u - ax) & 0xffffu;
                            const uint32_t bound = (negated - GP4_GLOBAL(uint16_t, 0x00603ba0)) & 0xffffu;
                            s.ecx = (s.ecx & 0xffff0000u) | bound;
                            flags = subFlags16(closing, bound);
                            run = static_cast<int16_t>(closing) >= static_cast<int16_t>(bound);
                        }
                        if (run) {
                            callRoutine(0x0042a278, s, flags);
                            if ((flags & 1u) == 0u) {
                                const uint32_t value = GP4_FIELD(uint8_t, car, 0x122) | 0x10u;
                                GP4_FIELD(uint8_t, car, 0x122) = static_cast<uint8_t>(value);
                                flags = logicFlags8(value);
                            }
                        }
                    }
                }
            }
        }
    }

    if (reset) {
        const uint32_t value = GP4_FIELD(uint8_t, car, 0x122) & 0xc1u;
        GP4_FIELD(uint8_t, car, 0x122) = static_cast<uint8_t>(value);
        flags = logicFlags8(value);

        gp4::Regs roll = s;
        callRoutine(0x00401462, roll, flags);
        const uint32_t eax = s.eax;
        s = roll;
        s.eax = eax;
        s.edx = roll.eax & 0xffu;
        flags = subFlags32(s.edx, 0x20u);
        if (static_cast<int32_t>(s.edx) >= 0x20) {
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            GP4_FIELD(uint32_t, car, 0xf0) = clock + 5000u;
            const uint32_t value2 = GP4_FIELD(uint8_t, car, 0x122) | 4u;
            GP4_FIELD(uint8_t, car, 0x122) = static_cast<uint8_t>(value2);
            flags = logicFlags8(value2);
        }
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->ebp = s.ebp;
    r->esi = s.esi;
    r->edi = s.edi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042f16d, FUN_0042f16d, "regs(esi:ptr[0x420]:layout=car_0042ba87+car_0042a278+rec_0042b84b+car_0042f16d, edi:ptr[0xb8]:layout=seg_0042d665+seg_0042f16d, eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00403800+state_004059ef+eq_0042ba87+state_0066d8c0+clock_0066d814+state_0042f16d")
