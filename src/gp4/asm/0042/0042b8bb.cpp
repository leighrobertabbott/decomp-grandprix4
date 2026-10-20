#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a TEST / logic op on an 8-bit result: CF = OF = AF = 0.
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a TEST / logic op on a 16-bit result.
static inline uint32_t testFlags16(uint32_t result) {
    result &= 0xffffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u);
}

// Flags of the 8-bit "cmp lhs, rhs".
static inline uint32_t cmpFlags8(uint32_t lhs, uint32_t rhs) {
    lhs &= 0xffu;
    rhs &= 0xffu;
    const uint32_t result = (lhs - rhs) & 0xffu;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit "cmp lhs, rhs" / "sub lhs, rhs".
static inline uint32_t cmpFlags16(uint32_t lhs, uint32_t rhs) {
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
static inline uint32_t cmpFlags32(uint32_t lhs, uint32_t rhs) {
    const uint32_t result = lhs - rhs;
    return (lhs < rhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((lhs ^ rhs) & (lhs ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0042b8bb
// esi = car, ebx = the other car (both records of the 22-car array at 0x66fef0). Decides the
// byte stored into 0x604155/0x604154/0x604153 (0, 0x80, the sign byte of the +0xea word
// difference, the byte 0x60440c, or what 0x0042adeb computes). ax = word 0x603e70; dh ends as
// the stored byte; the flags are those of the last flag writer on the path taken.
void __cdecl FUN_0042b8bb(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t otherAddr = r->ebx;

    r->eax = (r->eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00603e70);
    uint32_t edx = r->edx;
    uint32_t flags = 0u;
    bool callLookahead = false;   // 0x42ba6f: let 0x0042adeb decide

    const uint32_t carState = GP4_FIELD(uint16_t, car, 0xea);
    const uint32_t otherState = GP4_FIELD(uint16_t, other, 0xea);

    bool bothIdle = false;
    flags = cmpFlags16(carState, 0u);
    if (carState == 0u) {
        flags = cmpFlags16(otherState, 0u);
        bothIdle = (otherState == 0u);
    }

    if (bothIdle) {
        const uint32_t carFlags = GP4_FIELD(uint8_t, car, 0xfe);
        const uint32_t otherFlags = GP4_FIELD(uint8_t, other, 0xfe);
        flags = testFlags8(carFlags & 0x40u);
        if ((carFlags & 0x40u) == 0u) {
            flags = testFlags8(otherFlags & 0x40u);
            callLookahead = true;
        } else {
            flags = testFlags8(otherFlags & 0x40u);
            if ((otherFlags & 0x40u) == 0u) {
                callLookahead = true;
            } else {
                flags = testFlags8(carFlags & 0x10u);
                const uint32_t mark = (carFlags & 0x10u) != 0u ? 0x80u : 0u;
                edx = (edx & 0xffff00ffu) | (mark << 8);
                flags = testFlags8(otherFlags & 0x10u);
                const bool otherMarked = (otherFlags & 0x10u) != 0u;
                if (otherMarked == (mark != 0u)) {
                    callLookahead = true;
                } else {
                    GP4_GLOBAL(uint8_t, 0x00604155) = static_cast<uint8_t>(mark);
                }
            }
        }
    } else {
        bool forced = false;
        const uint32_t ctl = GP4_FIELD(uint8_t, car, 0x9c);
        flags = testFlags8(ctl & 2u);
        if ((ctl & 2u) != 0u) {
            forced = true;
        } else {
            const uint32_t bits = GP4_FIELD(uint8_t, car, 0x203);
            flags = testFlags8(bits & 8u);
            if ((bits & 8u) != 0u) {
                const uint32_t link = GP4_FIELD(uint32_t, car, 0x114);
                flags = cmpFlags32(otherAddr, link);
                forced = (otherAddr == link);
            }
        }

        if (forced) {
            const uint32_t preset = GP4_GLOBAL(uint8_t, 0x0060440c);
            flags = cmpFlags8(preset, 0u);
            if (preset != 0u) {
                edx = (edx & 0xffff00ffu) | (preset << 8);
                GP4_GLOBAL(uint8_t, 0x00604155) = static_cast<uint8_t>(preset);
            } else {
                const uint32_t mode = GP4_GLOBAL(uint16_t, 0x0086855c);
                flags = testFlags16(mode & 0xau);
                GP4_GLOBAL(uint8_t, 0x00604155) = (mode & 0xau) != 0u ? 0u : 0x80u;
            }
        } else {
            const uint32_t side = GP4_GLOBAL(uint8_t, 0x00603e6e);
            flags = testFlags8(side);
            if (side != 0u) {
                if ((side & 0x80u) == 0u) {
                    edx = (edx & 0xffff0000u) | carState;
                    flags = cmpFlags16(carState, otherState);
                    bool clear;
                    if (carState == otherState) {
                        const uint32_t steer = GP4_GLOBAL(uint16_t, 0x00604404);
                        flags = testFlags16(steer);
                        clear = (steer & 0x8000u) == 0u;
                    } else {
                        flags = cmpFlags16(carState, 0x20u);
                        const uint32_t dir = GP4_GLOBAL(uint8_t, 0x00868582);
                        flags = testFlags8(dir);
                        if (carState == 0x20u) {
                            clear = (dir & 0x80u) == 0u;
                        } else {
                            clear = (dir & 0x80u) != 0u;
                        }
                    }
                    GP4_GLOBAL(uint8_t, 0x00604155) = clear ? 0u : 0x80u;
                } else {
                    const uint32_t kind = GP4_FIELD(uint8_t, car, 0x8d);
                    flags = testFlags8(kind & 2u);
                    const uint32_t mode = GP4_GLOBAL(uint16_t, 0x0086855c);
                    flags = testFlags16(mode & 0xau);
                    const bool modeSet = (mode & 0xau) != 0u;
                    const bool kindSet = (kind & 2u) != 0u;
                    GP4_GLOBAL(uint8_t, 0x00604155) = (kindSet != modeSet) ? 0u : 0x80u;
                }
            } else {
                const uint32_t target = GP4_FIELD(uint16_t, car, 0x1bc);
                edx = target;
                flags = cmpFlags16(target, 0xffffu);
                bool linked = false;
                if (target != 0xffffu) {
                    edx = target + 0x0066fef0u;
                    flags = cmpFlags32(edx, otherAddr);
                    linked = (edx == otherAddr);
                }
                bool pass = true;
                if (!linked) {
                    const uint32_t carCe = GP4_FIELD(uint8_t, car, 0xce);
                    flags = testFlags8(carCe & 0x10u);
                    if ((carCe & 0x10u) != 0u) {
                        pass = false;
                    } else {
                        const uint32_t otherCe = GP4_FIELD(uint8_t, other, 0xce);
                        flags = testFlags8(otherCe & 0x10u);
                        if ((otherCe & 0x10u) != 0u) {
                            pass = false;
                        } else {
                            const uint32_t otherCc = GP4_FIELD(uint8_t, other, 0xcc);
                            flags = testFlags8(otherCc & 0x10u);
                            if ((otherCc & 0x10u) != 0u) pass = false;
                        }
                    }
                }
                if (pass) {
                    const uint32_t otherCe = GP4_FIELD(uint8_t, other, 0xce);
                    flags = testFlags8(otherCe & 0x10u);
                    if ((otherCe & 0x10u) != 0u) pass = false;
                }
                if (pass) {
                    const uint32_t diff = (carState - otherState) & 0xffffu;
                    flags = cmpFlags16(carState, otherState);
                    edx = (edx & 0xffff0000u) | diff;
                    GP4_GLOBAL(uint8_t, 0x00604155) = static_cast<uint8_t>(diff >> 8);
                    if (diff == 0u) callLookahead = true;
                } else {
                    callLookahead = true;
                }
            }
        }
    }

    if (callLookahead) {
        gp4::Regs s = *r;
        s.edx = edx;
        s.eflags = (r->eflags & ~0x8d5u) | flags;
        s.st_in = 0;
        s.st_out = 0;
        gp4::call_regs(0x0042adeb, s);
        flags = s.eflags & 0x8d5u;
    }

    const uint32_t result = GP4_GLOBAL(uint8_t, 0x00604155);
    edx = (edx & 0xffff00ffu) | (result << 8);
    GP4_GLOBAL(uint8_t, 0x00604154) = static_cast<uint8_t>(result);
    GP4_GLOBAL(uint8_t, 0x00604153) = static_cast<uint8_t>(result);
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042b8bb, FUN_0042b8bb, "regs(esi:rec[0x66fef0,0x41c,0..21], ebx:rec[0x66fef0,0x41c,0..21], eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_0066fe3c+cars_0066fef0+ring_008943a0+ring_00405a12+links_00405a12+state_0042bb90+bounds_0042e330+flags_00408f7b+pair_0042e8e6+link_0042a120+state_0042b8bb")
