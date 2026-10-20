#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit logic result (or / test): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

static inline int64_t mul64(uint32_t a, uint32_t b) {
    return static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
}

// `shrd eax, edx, n` after a 64-bit product: the low dword of product >> n.
static inline uint32_t lowShift(int64_t product, uint32_t n) {
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> n);
}

static inline uint32_t highDword(int64_t product) {
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
}

static inline uint8_t segmentFlags(uint32_t segment) {
    return GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(segment)), 0x51);
}

// eax = number of track segments to look ahead, edi = the car's 0x160-byte segment record (ring at
// 0x8943a0), esi = the car. Returns 0 at once for no segments, and 0 on the pit side when the
// segment is outside the window [0x868518] / [0x86851c] - 0x1600 (in front of [0x868518] the
// count is limited to the records up to it). Otherwise it steps forward by the count (pit end ->
// pit exit [0x866f04], pit entry [0x866f00] -> pit start for a car with +0x71 bit 7 inside the
// entry window, lap end -> - wrap) and walks back one record per iteration, keeping in ecx the
// largest scaled lane value |0x00427cd1| (speed +0x98 * [0x603854] >> 24, * lane >> 12,
// * [0x6f2bf0] >> 14) and, once nonzero, rescaling it by the speed estimate [0x624854] updated
// from the grip tables 0x62488c / 0x63a194 / 0x63acd4. The result is ecx * [0x624860] >> 14,
// at least [0x62480c] (set by 0x00428481). ebx, ecx, edx, ebp, esi, edi are saved and restored.
// FUNCTION: GP4 0x004284e1
void __cdecl FUN_004284e1(gp4::Regs* r) {
    const uint32_t count = r->eax;
    if (count == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(0u);
        return;
    }

    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    const uint32_t start = r->edi;
    uint32_t steps = count;
    if (start >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        const uint32_t gridEnd = GP4_GLOBAL(uint32_t, 0x00868518);
        if (start < gridEnd) {
            c.ecx = count;
            int32_t ahead = static_cast<int32_t>(gridEnd - start) / 0x160;
            if (ahead < 0) {
                ahead = 0;
            }
            if (ahead > static_cast<int32_t>(count)) {
                ahead = static_cast<int32_t>(count);
            }
            if (ahead == 0) {
                r->eax = 0u;
                r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(0u);
                return;
            }
            steps = static_cast<uint32_t>(ahead);
        } else {
            const uint32_t limit = GP4_GLOBAL(uint32_t, 0x0086851c) - 0x1600u;
            c.edx = limit;
            if (start < limit) {
                r->eax = 0u;
                r->eflags = (r->eflags & ~0x8d5u) | subFlags32(start, limit);
                return;
            }
        }
    }

    GP4_GLOBAL(uint32_t, 0x0062484c) = steps;
    GP4_GLOBAL(uint32_t, 0x00624850) = steps;
    GP4_GLOBAL(uint32_t, 0x00624868) = start;
    const uint32_t span = steps * 0x160u;
    uint32_t cursor = start + span;
    if (start >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        if (cursor >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            cursor = cursor - GP4_GLOBAL(uint32_t, 0x00866ef8) + GP4_GLOBAL(uint32_t, 0x00866f04);
        }
    } else {
        bool intoPit = false;
        if ((GP4_FIELD(uint8_t, car, 0x71) & 0x80u) != 0u
            && start >= GP4_GLOBAL(uint32_t, 0x00866f00) - 0x2100u
            && start < GP4_GLOBAL(uint32_t, 0x00866ef0)
            && cursor >= GP4_GLOBAL(uint32_t, 0x00866f00)) {
            cursor = cursor - GP4_GLOBAL(uint32_t, 0x00866f00) + GP4_GLOBAL(uint32_t, 0x00891500);
            intoPit = true;
        }
        if (!intoPit && cursor >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            cursor -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    }

    uint32_t speed = GP4_FIELD(uint32_t, car, 0x98);
    if (static_cast<int32_t>(speed) < 0x4000000) {
        speed = 0x4000000u;
    }
    GP4_GLOBAL(uint32_t, 0x00624854) = speed;
    GP4_GLOBAL(uint32_t, 0x00624884) = 0u;

    c.eax = speed;
    c.edi = cursor;
    gp4::call_regs(0x00428481, c);
    gp4::call_regs(0x00427d4b, c);
    c.ecx = 0u;

    for (;;) {
        if ((segmentFlags(cursor) & 0x80u) != 0u) {
            GP4_GLOBAL(uint32_t, 0x00624884) += GP4_GLOBAL(uint32_t, 0x00624880);
        } else {
            GP4_GLOBAL(uint32_t, 0x00624884) = 0u;
        }
        c.edi = cursor;
        gp4::call_regs(0x00427cd1, c);
        uint32_t lane = c.eax;
        if (static_cast<int32_t>(lane) < 0) {
            lane = 0u - lane;
        }
        c.eax = lane;
        GP4_GLOBAL(uint32_t, 0x0062486c) = lane;

        if (static_cast<int32_t>(lane) >= GP4_GLOBAL(int32_t, 0x00624864)) {
            GP4_GLOBAL(uint32_t, 0x00624850) = GP4_GLOBAL(uint32_t, 0x0062484c) - 1u;
            c.ebp = lane;
            uint32_t carSpeed = GP4_FIELD(uint32_t, car, 0x98);
            if ((segmentFlags(cursor) & 0x80u) != 0u) {
                carSpeed -= GP4_GLOBAL(uint32_t, 0x00624884);
            }
            if (static_cast<int32_t>(carSpeed) < 0x4000000) {
                carSpeed = 0x4000000u;
            }
            c.ebx = carSpeed;
            uint32_t value = lowShift(mul64(carSpeed, GP4_GLOBAL(uint32_t, 0x00603854)), 24);
            value = lowShift(mul64(value, lane), 12);
            const int64_t product = mul64(value, GP4_GLOBAL(uint32_t, 0x006f2bf0));
            value = lowShift(product, 14);
            c.edx = highDword(product);
            if (static_cast<int32_t>(value) < 0) {
                value = 0u - value;
            }
            c.eax = value;
            if (static_cast<int32_t>(value) > static_cast<int32_t>(c.ecx)) {
                c.ecx = value;
                GP4_GLOBAL(uint32_t, 0x00624854) = carSpeed;
            }
        }

        GP4_GLOBAL(uint32_t, 0x0062484c) -= 1u;
        if (GP4_GLOBAL(uint32_t, 0x0062484c) == 0u) {
            break;
        }

        const uint32_t pitStart = GP4_GLOBAL(uint32_t, 0x00891500);
        if (cursor < pitStart) {
            if (cursor == GP4_GLOBAL(uint32_t, 0x00866f04) && GP4_GLOBAL(uint32_t, 0x00624868) >= pitStart) {
                cursor = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            } else {
                cursor -= 0x160u;
                if (cursor < 0x008943a0u) {
                    cursor += GP4_GLOBAL(uint32_t, 0x00866ec4);
                }
            }
        } else if (cursor == pitStart) {
            cursor = cursor - pitStart + GP4_GLOBAL(uint32_t, 0x00866f00);
        } else {
            cursor -= 0x160u;
        }
        c.edi = cursor;

        if (c.ecx == 0u) {
            continue;
        }

        const uint32_t estimate = GP4_GLOBAL(uint32_t, 0x00624854);
        int32_t remainder;
        const uint32_t inverse = static_cast<uint32_t>(
            gp4::idiv64(static_cast<int64_t>(0x400) << 32, static_cast<int32_t>(estimate), &remainder));
        const uint32_t square = highDword(mul64(estimate, estimate));
        const int32_t steer = static_cast<int16_t>(static_cast<uint16_t>(
            GP4_FIELD(uint16_t, car, 0xc4) - GP4_FIELD(uint16_t, car, 0xe0)));
        const int64_t load = mul64(static_cast<uint32_t>(steer), square) >> 15;
        const uint32_t loadRate = static_cast<uint32_t>(
            gp4::idiv64(load, GP4_GLOBAL(int32_t, 0x00603854), &remainder));
        const uint32_t drag = lowShift(mul64(loadRate, GP4_GLOBAL(uint32_t, 0x0062d28c)), 14);

        uint32_t laneBand = GP4_GLOBAL(uint32_t, 0x0062486c) >> 5;
        if (static_cast<int32_t>(laneBand) > 0x3f) {
            laneBand = 0x3f;
        }
        uint32_t speedBand = estimate >> 24;
        if (static_cast<int32_t>(speedBand) > 0x3f) {
            speedBand = 0x3f;
        }
        const uint32_t grip = static_cast<uint32_t>(GP4_ARRAY(uint8_t, 0x0062488c)[speedBand | (laneBand << 6)]) << 6;
        const int32_t row = static_cast<int32_t>(estimate) >> 22;
        c.ebx = static_cast<uint32_t>(row);
        const uint32_t first = lowShift(mul64(GP4_ARRAY(uint32_t, 0x0063a194)[row], grip), 14);
        uint32_t limit = lowShift(mul64(GP4_ARRAY(uint32_t, 0x0063acd4)[row], GP4_GLOBAL(uint32_t, 0x00624888)), 14);
        if (static_cast<int32_t>(limit) > static_cast<int32_t>(first)) {
            limit = first;
        }
        uint32_t change = limit - drag;
        if (GP4_GLOBAL(uint32_t, 0x0062484c) == 1u) {
            int32_t weight = GP4_FIELD(int16_t, car, 0x66);
            if (weight < 0) {
                weight = 0;
            }
            if (weight > 0x4000) {
                weight = 0x4000;
            }
            change = lowShift(mul64(change, static_cast<uint32_t>(0x4000 - weight)), 14);
        }
        change = lowShift(mul64(change, GP4_FIELD(uint16_t, car, 0xd6)), 14);
        const int64_t scaled = mul64(change, inverse);
        uint32_t updated = static_cast<uint32_t>(scaled) + estimate;
        if (static_cast<int32_t>(updated) < 0x4000000) {
            updated = 0x4000000u;
        }
        GP4_GLOBAL(uint32_t, 0x00624854) = updated;
        const uint32_t gained = lowShift(mul64(updated, GP4_GLOBAL(uint32_t, 0x0062485c)), 14);
        const int32_t quotient = gp4::idiv64(mul64(gained, c.ecx), static_cast<int32_t>(estimate), &remainder);
        c.eax = static_cast<uint32_t>(quotient);
        c.ecx = static_cast<uint32_t>(quotient);
        c.edx = static_cast<uint32_t>(remainder);
        c.ebp = estimate;
    }

    uint32_t result = lowShift(mul64(c.ecx, GP4_GLOBAL(uint32_t, 0x00624860)), 14);
    const uint32_t floor = GP4_GLOBAL(uint32_t, 0x0062480c);
    const uint32_t flags = subFlags32(result, floor);
    if (static_cast<int32_t>(result) < static_cast<int32_t>(floor)) {
        result = floor;
    }
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004284e1, FUN_004284e1, "regs(eax:u32[0..28], edx:u32, edi:rec[0x8943a0,0x160,0..25], esi:ptr[0x2a0]:layout=car_0043c677+rec_00427880+car_004154d5) -> (eax, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+words_008943a0+walk_00427f3c+look_00428216+segw_00427bc1+line_008943a0+state_00427d4b+state_00427880+state_004154d5+state_00426947")
