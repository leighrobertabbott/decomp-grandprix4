#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

static inline uint16_t abs16(uint16_t value) {
    return static_cast<int16_t>(value) < 0 ? static_cast<uint16_t>(0u - value) : value;
}

// The original is "pushal ... popal": every general register is preserved and the flags are those of
// the last flag writer on the path taken.
static inline void finish(gp4::Regs* r, uint32_t flags) {
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}

// FUNCTION: GP4 0x00438548
// Car esi against segment record edi: unless the car is flagged (+0x1ce bit 1, or +0x7a bit 1 inside
// the record window [0x00866ed0, 0x00866ed4) with the 0x004384a3 test), compares the heading
// difference cosine (scaled 0x20b) plus |+0x90| with the car's +0xb0 + 0x100; when it reaches it and
// the lane-edge checks 0x00438029 allow, probes four directions with 0x004381ab, keeps the strongest
// (|[0x00629eac]| into 0x00629ec4, segment, offsets, heading word), and applies the response to the
// car flags, position and the 0x00438007 / 0x00437a74 / 0x00416387 / 0x00408742 helpers.
void __cdecl FUN_00438548(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t segment = r->edi;
    const bool isPlayer = r->esi == GP4_GLOBAL(uint32_t, 0x00679d30);
    if (isPlayer) {
        GP4_GLOBAL(uint8_t, 0x00625a28) = 0;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_FIELD(uint8_t, car, 0xcc) &= 0x7fu;
    GP4_FIELD(uint8_t, car, 0xe7) &= 0xfeu;

    bool flagged = (GP4_FIELD(uint8_t, car, 0x1ce) & 2u) != 0u;
    if (!flagged && (GP4_FIELD(uint8_t, car, 0x7a) & 2u) != 0u
        && segment >= GP4_GLOBAL(uint32_t, 0x00866ed0) && segment < GP4_GLOBAL(uint32_t, 0x00866ed4)) {
        const uint8_t high = static_cast<uint8_t>(GP4_FIELD(uint16_t, car, 0x90) >> 8);
        const uint8_t low = static_cast<uint8_t>(high ^ GP4_GLOBAL(uint8_t, 0x00868582));
        flagged = true;
        if ((low & 0x80u) != 0u) {
            c.eax = (r->eax & 0xffff0000u) | (static_cast<uint32_t>((high & 0x80u) != 0u ? 0xffu : 0u) << 8) | low;
            gp4::call_regs(0x004384a3, c);
            if (static_cast<int32_t>(c.eax) < 0x500) {
                finish(r, cmpFlags32(c.eax, 0x500u));
                return;
            }
        }
    }
    if (flagged) {
        gp4::call_regs(0x00408742, c);
        finish(r, c.eflags);
        return;
    }

    uint16_t angle = abs16(static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0x0e)
        - *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(segment))));
    if (angle >= 0x4000u) {
        angle = static_cast<uint16_t>(0x8000u - angle);
    }
    angle = abs16(static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(angle + 0x10a9u)));
    const uint32_t offset = static_cast<uint32_t>(angle >> 2) & 0xfffeu;
    const int32_t cosine = GP4_ARRAY(int16_t, 0x007c5c5c)[offset >> 1];
    const int64_t product = static_cast<int64_t>(cosine) * 0x20b;
    int32_t lateral = static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x90));
    if (lateral < 0) {
        lateral = -lateral;
    }
    GP4_GLOBAL(int32_t, 0x00629e98) = lateral;
    const uint32_t reach = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + static_cast<uint32_t>(lateral);
    GP4_GLOBAL(uint32_t, 0x00629ea0) = reach;
    const uint32_t limit = static_cast<uint32_t>(GP4_FIELD(uint16_t, car, 0xb0)) + 0x100u;
    if (static_cast<int32_t>(reach) < static_cast<int32_t>(limit)) {
        finish(r, cmpFlags32(reach, limit));
        return;
    }
    c.eax = reach;
    c.edx = limit;

    if (isPlayer) {
        GP4_GLOBAL(uint8_t, 0x00625a28) = 0x40;
    }

    if (GP4_GLOBAL(int32_t, 0x00629e98) < 0x6000) {
        c.edi = segment;
        gp4::call_regs(0x00438029, c);
        if ((c.eflags & 1u) == 0u) {
            c.edi = segment + 0x160u;
            if ((GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi)), 0x92) & 8u) != 0u) {
                gp4::call_regs(0x00406051, c);
            }
            gp4::call_regs(0x00438029, c);
            c.edi = segment;
            if ((c.eflags & 1u) == 0u) {
                if ((GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(segment)), 0x92) & 8u) != 0u) {
                    gp4::call_regs(0x00406077, c);
                } else {
                    c.edi = segment - 0x160u;
                }
                gp4::call_regs(0x00438029, c);
                c.edi = segment;
                if ((c.eflags & 1u) == 0u) {
                    finish(r, c.eflags);
                    return;
                }
            }
        }
    }

    if (isPlayer) {
        GP4_GLOBAL(uint8_t, 0x00625a28) = 0xff;
    }
    GP4_GLOBAL(int32_t, 0x00629ec4) = -1;
    for (uint32_t direction = 0; direction != 4u; ++direction) {
        c.eax = direction;
        c.ecx = direction;
        gp4::call_regs(0x004381ab, c);
        if ((c.eflags & 1u) == 0u) {
            continue;
        }
        int32_t strength = GP4_GLOBAL(int32_t, 0x00629eac);
        if (strength < 0) {
            strength = -strength;
        }
        if (strength < GP4_GLOBAL(int32_t, 0x00629ec4)) {
            continue;
        }
        GP4_GLOBAL(int32_t, 0x00629ec4) = strength;
        const uint32_t found = GP4_GLOBAL(uint32_t, 0x00629ec0);
        c.edi = found;
        GP4_GLOBAL(uint32_t, 0x00629ed8) = found;
        GP4_GLOBAL(uint32_t, 0x00629ed0) = GP4_GLOBAL(uint32_t, 0x00629eb8);
        GP4_GLOBAL(uint32_t, 0x00629ed4) = GP4_GLOBAL(uint32_t, 0x00629ebc);
        uint16_t heading;
        if (GP4_GLOBAL(uint8_t, 0x006279f8) != 0u) {
            uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(found));
            heading = GP4_FIELD(uint16_t, rec, 0x12c);
            if (GP4_FIELD(int16_t, car, 0x90) >= 0) {
                heading = GP4_FIELD(uint16_t, rec, 0x12e);
            }
        } else {
            heading = GP4_GLOBAL(uint16_t, 0x00629dd2);
        }
        GP4_GLOBAL(uint16_t, 0x00629f28) = heading;
    }

    if (GP4_GLOBAL(uint32_t, 0x00629ec4) == 0xffffffffu) {
        finish(r, cmpFlags32(0xffffffffu, 0xffffffffu));
        return;
    }
    GP4_GLOBAL(uint16_t, 0x00629dd2) = GP4_GLOBAL(uint16_t, 0x00629f28);
    c.edi = GP4_GLOBAL(uint32_t, 0x00629ed8);
    gp4::call_regs(0x0043883d, c);
    if ((c.eflags & 1u) != 0u) {
        const uint32_t threshold = GP4_GLOBAL(uint32_t, 0x0062d2a8) << 8;
        const uint32_t strength = GP4_GLOBAL(uint32_t, 0x00629ec4);
        if (static_cast<int32_t>(strength) < static_cast<int32_t>(threshold)) {
            finish(r, cmpFlags32(strength, threshold));
            return;
        }
        gp4::call_regs(0x004384e7, c);
    }

    GP4_FIELD(uint8_t, car, 0xcc) |= 0x80u;
    GP4_FIELD(uint8_t, car, 0x71) &= 0xfeu;
    GP4_FIELD(uint8_t, car, 0xe7) &= 0xfbu;
    if (GP4_FIELD(int16_t, car, 0x90) < 0) {
        GP4_FIELD(uint8_t, car, 0xe7) |= 4u;
    }
    uint16_t turn = GP4_FIELD(int16_t, car, 0x90) < 0 ? 0xc000u : 0x4000u;
    turn = abs16(static_cast<uint16_t>(static_cast<uint16_t>(turn + GP4_GLOBAL(uint16_t, 0x00629dd2))
        - GP4_FIELD(uint16_t, car, 0x0e)));
    if (static_cast<int16_t>(turn) < GP4_GLOBAL(int16_t, 0x00629f2e)) {
        GP4_FIELD(uint8_t, car, 0xe7) |= 1u;
    }
    GP4_FIELD(uint8_t, car, 0x163) |= 0x10u;
    GP4_FIELD(uint8_t, car, 0xce) |= 0x40u;
    gp4::call_regs(0x00438007, c);

    if ((GP4_FIELD(uint8_t, car, 0x7a) & 2u) != 0u) {
        const uint32_t strength = GP4_GLOBAL(uint32_t, 0x00629ec4);
        if (static_cast<int32_t>(strength) < 0x50000) {
            finish(r, cmpFlags32(strength, 0x50000u));
            return;
        }
        gp4::call_regs(0x00408742, c);
        finish(r, c.eflags);
        return;
    }

    GP4_FIELD(uint32_t, car, 0x00) -= GP4_GLOBAL(uint32_t, 0x00629ed0);
    GP4_FIELD(uint32_t, car, 0x04) -= GP4_GLOBAL(uint32_t, 0x00629ed4);
    gp4::call_regs(0x00437a74, c);
    const uint16_t speed = abs16(GP4_GLOBAL(uint16_t, 0x00629dce));
    const uint16_t minimum = GP4_GLOBAL(uint16_t, 0x00629dc6);
    if (static_cast<int16_t>(speed) < static_cast<int16_t>(minimum)) {
        finish(r, cmpFlags16(speed, minimum));
        return;
    }
    gp4::call_regs(0x00416387, c);
    finish(r, c.eflags);
}
GP4_IMPL(0x00438548, FUN_00438548, "regs(esi:ptr[0x420]:layout=car_004129fa+rec_00418c68+car_0043c677, edi:rec[0x8943a0,0x160,0..25]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_004317ae+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+segflag_004381ab+state_004381ab+state_00437d47+state_00418c68+state_0043247d+state_004125ce+flags_0066d909")
